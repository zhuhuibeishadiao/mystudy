#pragma once
namespace ddk::callback
{
	class nt_object_callback
	{
	public:
		using nt_object_pre_callback_type = std::function<VOID(POB_PRE_OPERATION_INFORMATION)>;
		using nt_object_post_callback_type = std::function<VOID(POB_POST_OPERATION_INFORMATION)>;
		nt_object_callback() {
			_ob_handler = nullptr;
		}
		~nt_object_callback() {
			if (_ob_handler)
			{
				ObUnRegisterCallbacks(_ob_handler);
			}
			_lock.wait_for_release();
		}
	private:
		ddk::nt_lock _lock;
		PVOID _ob_handler;
		std::vector<nt_object_pre_callback_type> _PreCallbacks;
		std::vector<nt_object_post_callback_type>_PostCallbacks;
	public:
		//我最喜欢obj的Altitude是320400,320200
		bool init(POBJECT_TYPE *_ObjectType,ULONG Altitude)
		{
			OB_CALLBACK_REGISTRATION  CallBackReg = {};
			OB_OPERATION_REGISTRATION OperationReg = {};

			
			auto ws_altitude = std::to_wstring(Altitude);

			CallBackReg.Version = ObGetFilterVersion();
			CallBackReg.OperationRegistrationCount = 1;
			CallBackReg.RegistrationContext = this;
			RtlInitUnicodeString(&CallBackReg.Altitude, ws_altitude.c_str());

			if (_ObjectType!=PsProcessType
				&&_ObjectType!=PsThreadType)
			{
				//其他objectType需要设置CallBackFlag!!!
				auto pObjectType = reinterpret_cast<ddk::ntos::build_7600::POBJECT_TYPE>(*_ObjectType);
				__try
				{
					pObjectType->TypeInfo.SupportsObjectCallbacks = 1;
				}
				__except (1)
				{
					return false;
				}
			}

			OperationReg.ObjectType = _ObjectType;
			OperationReg.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;

			OperationReg.PreOperation = (POB_PRE_OPERATION_CALLBACK)&(nt_object_callback::ObCallbackPreRoutine);
			OperationReg.PostOperation = (POB_POST_OPERATION_CALLBACK)&(nt_object_callback::ObCallbackPostRoutine);
			CallBackReg.OperationRegistration = &OperationReg;

			auto ns = ObRegisterCallbacks(&CallBackReg, &_ob_handler);
			if (NT_SUCCESS(ns))
			{
				return true;
			}

			ddk::status::LogStatus(ns);

			return false;
		}
		bool set_pre_callback(nt_object_pre_callback_type _pfn)
		{
			if (!_ob_handler)
				return false;
			_PreCallbacks.push_back(_pfn);
			return true;
		}
		bool set_post_callback(nt_object_post_callback_type _pfn)
		{
			if (!_ob_handler)
			{
				return false;
			}
			_PostCallbacks.push_back(_pfn);
			return true;
		}
	public:
		void _doPost(POB_POST_OPERATION_INFORMATION info)
		{
			_lock.only_acquire();
			std::for_each(_PostCallbacks.cbegin(),
				_PostCallbacks.cend(),
				[&](auto _pfn) {
				_pfn(info);
			}
			);
			_lock.release();
		}
		void _doPre(POB_PRE_OPERATION_INFORMATION info)
		{
			_lock.only_acquire();
			std::for_each(_PreCallbacks.cbegin(),
				_PreCallbacks.cend(),
				[&](auto _pfn) {
				_pfn(info);
			}
			);
			_lock.release();
		}
	public:
		static VOID ObCallbackPostRoutine(
			_In_ PVOID                          RegistrationContext,
			_In_ POB_POST_OPERATION_INFORMATION OperationInformation
		)
		{
			auto pThis = reinterpret_cast<nt_object_callback*>(RegistrationContext);
			__try
			{
				pThis->_doPost(OperationInformation);
			}
			__except (1)
			{

			}
		}
		static OB_PREOP_CALLBACK_STATUS ObCallbackPreRoutine(
			_In_ PVOID                         RegistrationContext,
			_In_ POB_PRE_OPERATION_INFORMATION OperationInformation
		)
		{
			auto pThis = reinterpret_cast<nt_object_callback*>(RegistrationContext);
			__try
			{
				pThis->_doPre(OperationInformation);
			}
			__except (1)
			{

			}
			return OB_PREOP_SUCCESS;
		}

	};
};