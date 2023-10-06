#include "astnode_constraint.h"
#include "astnode_type.h"
#include "define.h"
#include "type.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

#include "log.h"
#include "variable_table.h"
#include "verify_context.h"
#include <cassert>

AstNodeConstraint::AstNodeConstraint(std::string name, std::vector<std::string> generic_params, std::vector<ParserFnDeclare> rules) {
	m_name			 = name;
	m_generic_params = generic_params;
	m_rules			 = rules;
}
VerifyContextResult AstNodeConstraint::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify constraint");

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_name, new Variable(this));
	// 不同block内可以存在同名constraint, 只要他们没有构成隐藏
	// 为了区分同名constraint, 给每个名字分配一个唯一typeid
	m_result_typeid = g_typemgr.AddConstraint(m_name);
	return VerifyContextResult(TYPE_ID_NONE);
}
Variable* AstNodeConstraint::Execute(ExecuteContext& ctx) {
	return nullptr;
}
ConstraintInstance AstNodeConstraint::Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params, TypeId obj_tid) {
	//assert(HasGenericParam());
	if (concrete_params.size() != m_generic_params.size()) {
		panicf("wrong number of generic_params");
	}

	// 泛型名映射到实际类型id
	VariableTable* vt = new VariableTable();
	for (size_t i = 0; i < m_generic_params.size(); i++) {
		vt->AddVariable(m_generic_params.at(i), Variable::CreateTypeVariable(concrete_params.at(i)));
	}

	// 添加一个临时vt, 避免污染现有vt
	ctx.GetCurStack()->EnterBlock(vt);

	// 遍历所有rule模板, 生成实际类型
	for (auto rule : m_rules) {
		std::vector<TypeId> fn_params;

		// 处理方法的self参数
		if (!rule.param_list.empty()) {
			AstNodeType* first_param_type = rule.param_list.at(0).type;
			if (first_param_type->IsPointerSelf()) {
				// 函数第一个参数的类型是*self:  fn fn_name(*self, ...)
				if (!rule.param_list.at(0).name.empty()) {
					// self参数不能提供名字 (参数名字固定为self)
					panicf("self param should have empty param-name in constraint[%s.%s]", m_name.c_str(), rule.fnname.c_str());
				}
				TypeId ptr_obj_tid = g_typemgr.GetOrAddTypePointer(ctx, obj_tid);
				first_param_type->InitWithTargetTypeId(ptr_obj_tid);
			}
		}

		for (auto param : rule.param_list) {
			// 为了避免残留状态信息, 克隆一个临时type用作推导
			AstNodeType* astnode_type = param.type->DeepCloneT();
			TypeId		 param_tid	  = astnode_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
			fn_params.push_back(param_tid);
			delete astnode_type;
		}
		TypeId fn_return_tid = TYPE_ID_NONE;
		if (rule.return_type != nullptr) {
			// 为了避免残留状态信息, 克隆一个临时type用作推导
			AstNodeType* astnode_type = rule.return_type->DeepCloneT();
			fn_return_tid			  = astnode_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
			delete astnode_type;
		}
	}

	ctx.GetCurStack()->LeaveBlock();

	// 生成实例化后的constraint的唯一名字
	std::string constraint_instance_name = sprintf_to_stdstr("%s#%d[", m_name.c_str(), m_result_typeid);
	for (size_t i = 0; i < concrete_params.size(); i++) {
		constraint_instance_name += GET_TYPENAME(concrete_params.at(i));
		if (i + 1 < concrete_params.size()) {
			constraint_instance_name += ",";
		}
	}
	constraint_instance_name += "]";

	return ConstraintInstance{
		.constraint_name		  = m_name,
		.constraint_instance_name = constraint_instance_name,
	};
}
AstNodeConstraint* AstNodeConstraint::DeepCloneT() {
	AstNodeConstraint* newone = new AstNodeConstraint();

	newone->m_name			 = m_name;
	newone->m_generic_params = m_generic_params;
	for (auto iter : m_rules) {
		newone->m_rules.push_back(iter.DeepClone());
	}

	return newone;
}
