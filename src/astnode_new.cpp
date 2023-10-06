#include "astnode_new.h"
#include "astnode.h"
#include "astnode_compile_const.h"
#include "astnode_type.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_pointer.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Constants.h>
#include <llvm-12/llvm/IR/DataLayout.h>
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/IR/Type.h>
#include <llvm-12/llvm/IR/Value.h>
#include <llvm-12/llvm/Support/Alignment.h>
#include <llvm-12/llvm/Support/Casting.h>
#include <vector>

AstNodeNew::AstNodeNew(AstNodeType* ty, std::vector<AstNode*> args) {
	m_ty   = ty;
	m_args = args;

	m_ty->SetParent(this);
	for (auto& arg : m_args) {
		arg->SetParent(this);
	}
}

AstNodeNew::AstNodeNew(std::vector<std::string> ids, std::vector<AstNode*> args) {
	m_ty   = nullptr;
	m_ids  = ids;
	m_args = args;

	for (auto& arg : m_args) {
		arg->SetParent(this);
	}
}

/*
 * 
 */
VerifyContextResult AstNodeNew::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	if (vparam.ExpectLeftValue()) {
		panicf("`new expr` cannot be lvalue");
	}

	if (m_ty != nullptr) {
		VerifyContextResult vr_expr		= m_ty->Verify(ctx, VerifyContextParam());
		TypeId				pointee_tid = TYPE_ID_NONE;
		TypeInfo*			ti_ty		= g_typemgr.GetTypeInfo(vr_expr.GetResultTypeId());
		if (ti_ty->IsArray()) {
			// 是数组, new表达式的结果是*ElementType. 将size_expr视作一个参数
			// new [size_expr]T(init_expr_optional);
			TypeInfoArray* ti_array = dynamic_cast<TypeInfoArray*>(ti_ty);
			pointee_tid				= ti_array->GetElementType();

			if (m_args.empty()) {
				// new [size_expr]T();
			} else if (m_args.size() == 1) {
				// new [size_expr]T(init_expr);
				TypeId				tid_ptr_to_element = g_typemgr.GetOrAddTypePointer(ctx, pointee_tid);
				VerifyContextResult vr_result		   = m_args.at(0)->Verify(ctx, VerifyContextParam().SetExpectResultTid(tid_ptr_to_element));
				if (vr_result.GetResultTypeId() != tid_ptr_to_element) {
					panicf("arg type should be [%d:%s]. give[%d:%s]", tid_ptr_to_element, GET_TYPENAME_C(tid_ptr_to_element), vr_result.GetResultTypeId(), GET_TYPENAME_C(vr_result.GetResultTypeId()));
				}
			} else {
				panicf("syntax error");
			}
		} else {
			pointee_tid = ti_ty->GetTypeId();
			// new T();
			// new T(T);
			if (m_args.empty()) {
			} else if (m_args.size() == 1) {
				VerifyContextResult vr_arg = m_args.at(0)->Verify(ctx, VerifyContextParam().SetExpectResultTid(pointee_tid));
				if (vr_arg.GetResultTypeId() != pointee_tid) {
					panicf("argtype should be[%d:%s]. give[%d:%s]", pointee_tid, GET_TYPENAME_C(pointee_tid), vr_arg.GetResultTypeId(), GET_TYPENAME_C(vr_arg.GetResultTypeId()));
				}
			} else {
				panicf("invalid new_expr syntax");
			}
		}

		m_result_typeid = g_typemgr.GetOrAddTypePointer(ctx, pointee_tid);

	} else {
		// new Foo(...)
		// new Foo.Bar(...)
		assert(m_ids.size() > 0 && m_ids.size() <= 2);
		Variable* ty = ctx.GetCurStack()->GetVariable(m_ids.at(0));
		if (ty->GetTypeId() != TYPE_ID_TYPE) {
			panicf("[%s] is not type", m_ids.at(0).c_str());
		}
		TypeInfo* ti_class = g_typemgr.GetTypeInfo(ty->GetValueTid());

		std::string constructor_fnname;
		if (m_ids.size() == 1) {
			constructor_fnname = m_ids.at(0);
		} else if (m_ids.size() == 2) {
			constructor_fnname = m_ids.at(1);
		} else {
			panicf("not implemented yet");
		}

		if (ti_class->IsSimpleConstrcutor(constructor_fnname)) {
			std::vector<std::string> constructors = ti_class->GetConstructor(ctx, constructor_fnname);
			if (constructors.empty()) {
				panicf("no candidate constructor match");
			} else if (constructors.size() > 1) {
				panicf("multiple candidates constructor match");
			} else {
				m_constructor_fnid = constructors.at(0);
			}
			TypeId		fn_tid = ctx.GetFnTable().GetFnTypeId(m_constructor_fnid);
			TypeInfoFn* ti_fn  = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));
			if (ti_fn->GetParamNum() != m_args.size() + 1) {
				panicf("no candidate constructor match");
			}
			for (size_t i = 0; i < m_args.size(); i++) {
				TypeId				arg_expect_tid = ti_fn->GetParamType(i + 1);
				VerifyContextResult vr_arg		   = m_args.at(i)->Verify(ctx, VerifyContextParam().SetExpectResultTid(arg_expect_tid));
				if (vr_arg.GetResultTypeId() != arg_expect_tid) {
					panicf("bug");
				}
			}
		} else {
			std::vector<TypeId> args_tid;
			for (auto& arg : m_args) {
				args_tid.push_back(arg->Verify(ctx, VerifyContextParam()).GetResultTypeId());
			}

			std::vector<std::string> constructors = ti_class->GetConstructor(ctx, constructor_fnname, args_tid);
			if (constructors.empty()) {
				panicf("no candidate constructor match");
			} else if (constructors.size() > 1) {
				panicf("multiple candidates constructor match");
			} else {
				m_constructor_fnid = constructors.at(0);
			}
		}

		m_result_typeid = g_typemgr.GetOrAddTypePointer(ctx, ti_class->GetTypeId());
	}
	return VerifyContextResult(m_result_typeid);
}
AstNodeNew* AstNodeNew::DeepCloneT() {
	std::vector<AstNode*> new_args;
	for (auto& arg : m_args) {
		new_args.push_back(arg->DeepClone());
	}
	if (m_ty == nullptr) {
		AstNodeNew* newone = new AstNodeNew(m_ids, new_args);
		newone->Copy(*this);
		return newone;
	} else {
		AstNodeNew* newone = new AstNodeNew(m_ty->DeepCloneT(), new_args);
		newone->Copy(*this);
		return newone;
	}
}
CompileResult AstNodeNew::Compile(CompileContext& cctx) {
	if (m_compile_to_left_value) {
		panicf("`new expr` cannot be lvalue");
	}

	TypeInfoPointer* ti				 = dynamic_cast<TypeInfoPointer*>(g_typemgr.GetTypeInfo(m_result_typeid));
	llvm::Type*		 ir_type_pointer = ti->GetLLVMIRType(cctx);
	TypeId			 pointee_tid	 = ti->GetPointeeTid();
	TypeInfo*		 ti_pointee		 = g_typemgr.GetTypeInfo(pointee_tid);
	llvm::Type*		 ir_type_pointee = ti_pointee->GetLLVMIRType(cctx);

	llvm::Function* fn_malloc = IRM.getFunction("malloc");

	llvm::Value* ret = nullptr;

	if (m_ty != nullptr && m_ty->IsArrayType()) {
		// new [s]T();
		CompileResult cr_array_size = m_ty->GetArraySizeExpr()->Compile(cctx);
		assert(cr_array_size.GetResult()->getType() == g_typemgr.GetTypeInfo(TYPE_ID_UINT64)->GetLLVMIRType(cctx));
		llvm::Value* malloc_size = IRB.CreateMul(cr_array_size.GetResult(), llvm::ConstantExpr::getSizeOf(ir_type_pointee));
		llvm::Value* inst		 = IRB.CreateCall(fn_malloc, std::vector<llvm::Value*>{malloc_size});

		// malloc函数返回的是*i8, 转会为对应指针类型
		llvm::Value* cast_inst = IRB.CreateBitCast(inst, ir_type_pointer);

		if (m_args.size() == 1) {
			CompileResult cr_init_data = m_args.at(0)->Compile(cctx);
			IRB.CreateMemCpy(cast_inst, llvm::MaybeAlign(8), cr_init_data.GetResult(), llvm::MaybeAlign(8), malloc_size);
		}
		ret = cast_inst;
	} else if (is_integer_type(pointee_tid) || pointee_tid == TYPE_ID_BOOL || pointee_tid == TYPE_ID_FLOAT32 || pointee_tid == TYPE_ID_FLOAT64) {
		// new T();
		// new T(T);
		llvm::Value* inst = IRB.CreateCall(fn_malloc, std::vector<llvm::Value*>{llvm::ConstantExpr::getSizeOf(ir_type_pointee)});
		// malloc函数返回的是*i8, 转会为对应指针类型
		llvm::Value* cast_inst = IRB.CreateBitCast(inst, ir_type_pointer);
		if (m_args.empty()) {
		} else if (m_args.size() == 1) {
			CompileResult cr_init_value = m_args.at(0)->Compile(cctx);
			assert(cr_init_value.GetResult()->getType() == ir_type_pointee);
			IRB.CreateStore(cr_init_value.GetResult(), cast_inst);
		} else {
			panicf("bug");
		}
		ret = cast_inst;
	} else {
		// new class_type(...);
		llvm::Value* inst = IRB.CreateCall(fn_malloc, std::vector<llvm::Value*>{llvm::ConstantExpr::getSizeOf(ir_type_pointee)});
		// malloc函数返回的是*i8, 转会为对应指针类型
		llvm::Value* cast_inst = IRB.CreateBitCast(inst, ir_type_pointer);
		// 调用构造函数
		std::vector<llvm::Value*> constructor_args;
		constructor_args.push_back(cast_inst);
		for (auto& arg : m_args) {
			constructor_args.push_back(arg->Compile(cctx).GetResult());
		}
		llvm::Function* fn_constructor = IRM.getFunction(m_constructor_fnid);
		IRB.CreateCall(fn_constructor, constructor_args);
		ret = cast_inst;
	}

	return CompileResult().SetResult(ret);
}
