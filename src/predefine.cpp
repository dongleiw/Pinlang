#include "predefine.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

// void* malloc(uint64_t size);
// libc
//static AstNodeComplexFnDef* define_builin_fn_malloc() {
//	std::vector<AstNodeComplexFnDef::Implement> implements;
//
//	AstNodeType* param0_type = new AstNodeType();
//	param0_type->InitWithTargetTypeId(TYPE_ID_UINT64);
//
//	std::vector<ParserGenericParam> gparams;
//	std::vector<ParserParameter>	params{ParserParameter{
//		   .name = "size",
//		   .type = param0_type,
//	   }};
//
//	AstNodeType* return_type = new AstNodeType();
//	return_type->InitWithIdentifier("T" + idx);
//	params.push_back({ParserParameter{
//		.name = "a" + idx,
//		.type = value_type,
//	}});
//	return AstNodeComplexFnDef::Implement(gparams, params, nullptr, builtin_fn_printf_verify);
//
//	AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("printf", implements);
//
//	return astnode_complex_fndef;
//}
void register_predefine(AstNodeBlockStmt& astnode_block_stmt) {
}
