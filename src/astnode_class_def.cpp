#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_class.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

// TODO 检查字段名和方法名不重复
VerifyContextResult AstNodeClassDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify class def[%s]", m_class_name.c_str());

	if (ctx.GetCurStack()->IsVariableExist(m_class_name)) {
		panicf("conflict class name[%s]", m_class_name.c_str());
	}

	TypeInfoClass* ti = new TypeInfoClass(m_class_name);
	m_result_typeid	  = g_typemgr.AddTypeInfo(ti);
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_class_name, Variable::CreateTypeVariable(m_result_typeid));

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
	for (auto iter : m_method_list) {
		iter->Verify(ctx, VerifyContextParam());
	}
	ti->AddConstraint(CONSTRAINT_ID_NONE, m_method_list);

	// 检查实现的约束
	// TODO 检查实现的函数是否和constraint声明的一致, 是否有遗漏
	for (auto impl : m_impl_constraint_list) {
		std::vector<TypeId> gparam_tids;
		for (auto gparam_type : impl.constraint_gparams) {
			gparam_tids.push_back(gparam_type->Verify(ctx, VerifyContextParam()).GetResultTypeId());
		}
		AstNodeConstraint* astnode_constraint	   = ctx.GetCurStack()->GetVariable(impl.constraint_name)->GetValueConstraint();
		TypeId			   constraint_instance_tid = astnode_constraint->Instantiate(ctx, gparam_tids);

		for (auto fn : impl.constraint_fns) {
			ctx.GetCurStack()->EnterBlock(new VariableTable());
			fn->Verify(ctx, VerifyContextParam());
			ctx.GetCurStack()->LeaveBlock();
		}
		ti->AddConstraint(constraint_instance_tid, impl.constraint_fns);
	}

	ctx.GetCurStack()->LeaveBlock();

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeClassDef::Execute(ExecuteContext& ctx) {
	return nullptr;
}
AstNodeClassDef* AstNodeClassDef::DeepCloneT() {
	AstNodeClassDef* newone = new AstNodeClassDef();

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
