#include "type_bool.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

#include <cassert>


TypeInfoBool::TypeInfoBool() {
	m_name			 = "bool";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
	m_mem_size		 = 1;
	m_mem_align_size = 1;
	m_is_value_type	 = true;
}
void TypeInfoBool::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	ctx.PopSTack();
}
llvm::Type* TypeInfoBool::GetLLVMIRType(CompileContext& cctx) {
	return llvm::Type::getInt1Ty(IRC);
}
