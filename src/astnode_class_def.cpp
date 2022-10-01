#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "function.h"
#include "type.h"
#include "type_class.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "variable_table.h"
#include "verify_context.h"

// TODO 检查字段名和方法名不重复
VerifyContextResult AstNodeClassDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify class def[%s]", m_class_name.c_str());

	ctx.GetCurStack()->EnterBlock(new VariableTable());

	TypeInfoClass* ti = new TypeInfoClass(m_class_name);
	m_result_typeid = g_typemgr.AddTypeInfo(ti);

	// 检查类的字段
	for (auto iter : m_field_list) {
		if (ti->HasField(iter.name)) {
			panicf("field[%s] already exists", iter.name.c_str());
		}
		TypeId field_tid = iter.type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		ti->AddField(iter.name, field_tid);
	}

	// 检查类的方法
	for(auto iter:m_method_list){
		iter->Verify(ctx, VerifyContextParam());
	}
	ti->AddConstraint(CONSTRAINT_ID_NONE, m_method_list);

	ctx.GetCurStack()->LeaveBlock();

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_class_name, Variable::CreateTypeVariable(m_result_typeid));

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
