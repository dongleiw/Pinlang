#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include <llvm-12/llvm/IR/Function.h>
#include <string>

#include "astnode_complex_fndef.h"
#include "fntable.h"
#include "type_mgr.h"

void BuiltinFn::register_builtin_fns(AstNodeBlockStmt& astnode_block_stmt) {
	astnode_block_stmt.AddChildStmt(register_bfn_abort());
	astnode_block_stmt.AddChildStmt(register_bfn_write());
}
AstNodeComplexFnDef* BuiltinFn::register_bfn_abort() {
	const std::string				fnid = "abort";
	std::vector<ParserGenericParam> gparams;
	std::vector<ParserParameter>	params{};

	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, nullptr, BuiltinFn::compile_bfn_abort));
	return new AstNodeComplexFnDef("abort", implements, FnAttr::FN_ATTR_NONE);
}
void BuiltinFn::compile_bfn_abort(CompileContext& cctx, const FnInfo& fn_info) {
	llvm::FunctionType* libc_abort_fn_type = llvm::FunctionType::get(llvm::Type::getVoidTy(IRC), std::vector<llvm::Type*>{}, false);
	llvm::Function*		libc_abort_fn	   = llvm::Function::Create(libc_abort_fn_type, llvm::Function::ExternalLinkage, "abort", IRM);

	cctx.AddNamedValue(fn_info.GetFnId(), libc_abort_fn);
}
AstNodeComplexFnDef* BuiltinFn::register_bfn_write() {
	const std::string				fnid = "write";
	std::vector<ParserGenericParam> gparams;
	std::vector<ParserParameter>	params{};
	{
		AstNodeType* fd_type = new AstNodeType();
		fd_type->InitWithTargetTypeId(TYPE_ID_INT32);
		params.push_back({ParserParameter{
			.name = "fd",
			.type = fd_type,
		}});
	}
	{
		AstNodeType* pointee_type = new AstNodeType();
		pointee_type->InitWithTargetTypeId(TYPE_ID_UINT8);
		AstNodeType* data_type = new AstNodeType();
		data_type->InitWithPointer(pointee_type);
		params.push_back({ParserParameter{
			.name = "data",
			.type = data_type,
		}});
	}
	{
		AstNodeType* size_type = new AstNodeType();
		size_type->InitWithTargetTypeId(TYPE_ID_UINT64);
		params.push_back({ParserParameter{
			.name = "size",
			.type = size_type,
		}});
	}

	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, nullptr, BuiltinFn::compile_bfn_write));
	return new AstNodeComplexFnDef("write", implements, FnAttr::FN_ATTR_NONE);
}

void BuiltinFn::compile_bfn_write(CompileContext& cctx, const FnInfo& fn_info) {
	llvm::FunctionType* libc_write_fn_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(IRC), std::vector<llvm::Type*>{
																									  llvm::Type::getInt32Ty(IRC),
																									  llvm::Type::getInt8PtrTy(IRC),
																									  llvm::Type::getInt64Ty(IRC),
																								  },
																	 false);
	llvm::Function*		libc_write_fn	   = llvm::Function::Create(libc_write_fn_type, llvm::Function::ExternalLinkage, "write", IRM);

	cctx.AddNamedValue(fn_info.GetFnId(), libc_write_fn);
}
void BuiltinFn::compile_nop(CompileContext& cctx, const FnInfo& fn_info) {
}
void BuiltinFn::register_external_fn(CompileContext& cctx) {
	llvm::FunctionType* libc_malloc_fn_type = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(IRC), std::vector<llvm::Type*>{llvm::Type::getInt64Ty(IRC)}, false);
	llvm::Function*		libc_malloc_fn		= llvm::Function::Create(libc_malloc_fn_type, llvm::Function::ExternalLinkage, "malloc", IRM);
}
void BuiltinFn::compile_bfn_tostring(CompileContext& cctx, const FnInfo& fn_info) {
	assert(fn_info.GetFnName() == "tostring");
	assert(fn_info.IsMethod() && !fn_info.IsConstructor());
	assert(fn_info.GetParamNum() == 1 && fn_info.GetParams().at(0).IsSelfParam() && fn_info.GetParams().at(0).GetParamName() == "self");

	// 生成对应integer类型的tostring方法
	// 先临时调用libc的printf方法将integer转换为string
	TypeInfo*	ti_obj		= g_typemgr.GetTypeInfo(fn_info.GetObjTid());
	llvm::Type* ir_type_obj = ti_obj->GetLLVMIRType(cctx);

	// 1. integer最长20个字符(包括正负号前缀). 现在heap上申请一块21个byte的内存(包括'\0')
	// 2. 调用sprintf将integer转换为string存储到申请的内存中
	// 3. 构建一个Str
	switch (fn_info.GetObjTid()) {
	case TYPE_ID_INT8:
		break;
	case TYPE_ID_UINT8:
		break;
	case TYPE_ID_INT16:
		break;
	case TYPE_ID_UINT16:
		break;
	case TYPE_ID_INT32:
		break;
	case TYPE_ID_UINT32:
		break;
	case TYPE_ID_INT64:
		break;
	case TYPE_ID_UINT64:
		break;
	case TYPE_ID_FLOAT32:
		break;
	case TYPE_ID_FLOAT64:
		break;
	case TYPE_ID_BOOL:
		break;
	default:
		panicf("unknown literal type[%d]", fn_info.GetObjTid());
		break;
	}
}
