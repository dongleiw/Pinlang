#include "type_pointer.h"
#include "define.h"
#include "type_mgr.h"
#include "utils.h"

TypeInfoPointer::TypeInfoPointer(TypeId pointee_tid) {
	m_typegroup_id = TYPE_GROUP_ID_POINTER;
	m_pointee_tid  = pointee_tid;
	m_name		   = sprintf_to_stdstr("*%s", GET_TYPENAME_C(pointee_tid));
}
void TypeInfoPointer::InitBuiltinMethods(VerifyContext& ctx) {
}
llvm::Type* TypeInfoPointer::GetLLVMIRType(CompileContext& cctx) {
	TypeInfo* ti_pointee = g_typemgr.GetTypeInfo(m_pointee_tid);
	return ti_pointee->GetLLVMIRType(cctx)->getPointerTo();
}
