#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "instruction.h"
#include "source_info.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * new T( expr_list_optional )
 * new id1.id2( expr_list_optional )
 */
class AstNodeNew : public AstNode {
public:
	AstNodeNew(AstNodeType* ty, std::vector<AstNode*> args);
	AstNodeNew(std::vector<std::string> ids, std::vector<AstNode*> args);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	AstNodeNew*		 DeepCloneT();
	virtual AstNode* DeepClone() override { return DeepCloneT(); }

private:
	AstNodeType*			 m_ty;
	std::vector<std::string> m_ids;
	std::vector<AstNode*>	 m_args;

	std::string m_constructor_fnid;
};
