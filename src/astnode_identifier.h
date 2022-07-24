#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 操作符: + - * / %
 * 操作符可以转换为方法调用, 但是目前还没不支持方法
 */
class AstNodeIdentifier : public AstNode {
  public:
	AstNodeIdentifier(std::string id): m_id(id){
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

  private:
	std::string m_id;
};
