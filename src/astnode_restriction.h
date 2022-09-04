#pragma once

#include <string>
#include <vector>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 类型约束. 对类型的限定条件的集合
 */
class AstNodeRestriction : public AstNode {
public:
	AstNodeRestriction(TypeId restriction_tid) : m_restriction_tid(restriction_tid){
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

private:
	TypeId m_restriction_tid;
};
