#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "compile_context.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_class.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/DerivedTypes.h>

// TODO 检查字段名和方法名不重复
VerifyContextResult AstNodeClassDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify class def[%s]", m_class_name.c_str());

	if (ctx.GetCurStack()->IsVariableExist(m_class_name)) {
		panicf("conflict class name[%s]", m_class_name.c_str());
	}

	m_class_tid = g_typemgr.AddTypeClass(new TypeInfoClass(m_class_name));
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_class_name, Variable::CreateTypeVariable(m_class_tid));
	TypeInfoClass* ti = dynamic_cast<TypeInfoClass*>(g_typemgr.GetTypeInfo(m_class_tid));

	ctx.GetCurStack()->EnterBlock(new VariableTable());

	// 检查类的字段
	std::vector<std::pair<std::string, TypeId>> fields;
	for (auto iter : m_field_list) {
		if (ti->HasField(iter.name)) {
			panicf("field[%s] already exists", iter.name.c_str());
		}
		TypeId field_tid = iter.type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		fields.push_back(std::pair(iter.name, field_tid));
	}
	ti->SetFields(fields);

	// 检查类的普通(非约束)方法
	for (auto method : m_method_list) {
		method->SetObjTypeId(m_class_tid);
		if (method->GetName() == m_class_name) {
			method->SetFnAttr((FnAttr)(method->GetFnAttr() | FnAttr::FN_ATTR_CONSTRUCTOR));
		}
		method->Verify(ctx, VerifyContextParam());
	}
	ti->AddConstraint(ConstraintInstance{}, m_method_list);

	if (!ti->HasConstructor()) {
		// 如果没有定义, 添加默认构造函数
		ti->AddDefaultConstructor(ctx);
	}
	std::vector<std::string> constructors = ti->GetConstructor(ctx, ti->GetOriginalName(), std::vector<TypeId>{});
	if (constructors.empty()) {
	} else if (constructors.size() > 1) {
		panicf("multiple candidates constructor match");
	} else {
		ti->SetDefaultConstrutorFnId(constructors.at(0));
	}

	// 检查实现的约束
	// TODO 检查实现的函数是否和constraint声明的一致, 是否有遗漏
	for (auto impl : m_impl_constraint_list) {
		std::vector<TypeId> gparam_tids;
		for (auto gparam_type : impl.constraint_gparams) {
			gparam_tids.push_back(gparam_type->Verify(ctx, VerifyContextParam()).GetResultTypeId());
		}
		Variable* constraint = ctx.GetCurStack()->GetVariableOrNull(impl.constraint_name);
		if (constraint == nullptr) {
			ctx.VerifyGlobalIdentifier(this, impl.constraint_name, VerifyContextParam());
			constraint = ctx.GetCurStack()->GetVariableOrNull(impl.constraint_name);
			if (constraint == nullptr) {
				panicf("unknown constraint[%s]", impl.constraint_name.c_str());
			}
		}

		AstNodeConstraint*					  astnode_constraint  = constraint->GetValueConstraint();
		ConstraintInstance constraint_instance = astnode_constraint->Instantiate(ctx, gparam_tids, m_class_tid);

		for (auto fn : impl.constraint_fns) {
			fn->SetObjTypeId(m_class_tid);
			ctx.GetCurStack()->EnterBlock(new VariableTable());
			fn->Verify(ctx, VerifyContextParam());
			ctx.GetCurStack()->LeaveBlock();
		}
		ti->AddConstraint(constraint_instance, impl.constraint_fns);
	}

	ctx.GetCurStack()->LeaveBlock();

	return VerifyContextResult(TYPE_ID_NONE);
}
Variable* AstNodeClassDef::Execute(ExecuteContext& ctx) {
	return nullptr;
}
AstNodeClassDef* AstNodeClassDef::DeepCloneT() {
	AstNodeClassDef* newone = new AstNodeClassDef();
	newone->Copy(*this);

	newone->m_class_name = m_class_name;
	for (auto iter : m_field_list) {
		newone->m_field_list.push_back(iter.DeepClone());
	}
	for (auto iter : m_method_list) {
		newone->m_method_list.push_back(iter->DeepCloneT());
	}
	for (auto iter : m_subclass_list) {
		newone->m_subclass_list.push_back(iter->DeepClone());
	}

	return newone;
}
CompileResult AstNodeClassDef::Compile(CompileContext& cctx) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_class_tid);
	ti->GetLLVMIRType(cctx);
	return CompileResult();
}
