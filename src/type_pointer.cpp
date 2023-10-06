#include "type_pointer.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "type_mgr.h"
#include "utils.h"
#include <llvm-12/llvm/IR/Constants.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>

TypeInfoPointer::TypeInfoPointer(TypeId pointee_tid) {
	m_typegroup_id	= TYPE_GROUP_ID_POINTER;
	m_pointee_tid	= pointee_tid;
	m_original_name = sprintf_to_stdstr("*%s", GET_TYPENAME_C(pointee_tid));
	m_name			= sprintf_to_stdstr("*%s", GET_TYPENAME_C(pointee_tid));
}
void TypeInfoPointer::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());

	// 手动实现Index约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* index_type = new AstNodeType();
					index_type->InitWithIdentifier("u64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = index_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithTargetTypeId(m_pointee_tid);

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("index", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_pointee_tid); // 由于自动解引用. 所以指针的方法的this参数类型是pointee_type
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Index")->GetValueConstraint();
		ConstraintInstance constraint_instance= constraint->Instantiate(ctx, std::vector<TypeId>{m_pointee_tid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}

	ctx.PopStack();
}
llvm::Type* TypeInfoPointer::GetLLVMIRType(CompileContext& cctx) {
	TypeInfo* ti_pointee = g_typemgr.GetTypeInfo(m_pointee_tid);
	return ti_pointee->GetLLVMIRType(cctx)->getPointerTo();
}
void TypeInfoPointer::ConstructDefault(CompileContext& cctx, llvm::Value* obj) {
	llvm::Type* ty = GetLLVMIRType(cctx);
	assert(obj->getType() == ty->getPointerTo());
	assert(ty->isPointerTy());
	// 构造一个类型正确的null指针, 将obj初始化为null
	IRB.CreateStore(llvm::ConstantPointerNull::get((llvm::PointerType*)ty), obj);
}
