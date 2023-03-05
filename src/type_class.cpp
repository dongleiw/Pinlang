#include "type_class.h"

#include <assert.h>
#include <map>
#include <vector>

#include "astnode_constraint.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

TypeInfoClass::TypeInfoClass(std::string class_name) {
	m_name		   = class_name;
	m_typegroup_id = TYPE_GROUP_ID_CLASS;
	m_ir_type	   = nullptr;
}
void TypeInfoClass::InitBuiltinMethods(VerifyContext& ctx) {
}
llvm::Type* TypeInfoClass::GetLLVMIRType(CompileContext& cctx) {
	if (m_ir_type == nullptr) {
		std::vector<llvm::Type*> ir_type_fields;
		for (auto f : m_field_list) {
			TypeInfo* ti_field = g_typemgr.GetTypeInfo(f.tid);
			ir_type_fields.push_back(ti_field->GetLLVMIRType(cctx));
		}
		m_ir_type = llvm::StructType::create(ir_type_fields, m_name);
	}
	return m_ir_type;
}
