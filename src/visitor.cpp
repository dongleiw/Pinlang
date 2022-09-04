#include "visitor.h"

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_fncall.h"
#include "astnode_fndef.h"
#include "astnode_identifier.h"
#include "astnode_literal.h"
#include "astnode_operator.h"
#include "astnode_restriction.h"
#include "astnode_return.h"
#include "astnode_vardef.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_restriction.h"
#include "utils.h"
#include <any>
#include <ios>
#include <memory>

/*
 * 解析类型. 返回TypeId
 */
std::any Visitor::visitType(PinlangParser::TypeContext* ctx) {
	if (ctx->TYPE() != nullptr) {
		return TYPE_ID_TYPE;
	} else if (ctx->Identifier() != nullptr) {
		std::string type_name = ctx->Identifier()->getText();
		TypeId		tid		  = g_typemgr.GetTypeIdByName_or_unresolve(type_name);
		return tid;
	} else {
		panicf("unknown type");
	}
	return nullptr;
}
std::any Visitor::visitExpr_primary_literal(PinlangParser::Expr_primary_literalContext* ctx) {
	auto literal = ctx->literal();
	if (literal->IntegerLiteral() != nullptr) {
		int value = str_to_int(literal->IntegerLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else if (literal->FloatLiteral() != nullptr) {
		float value = str_to_float(literal->FloatLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else if (literal->StringLiteral() != nullptr) {
		std::string value = literal->StringLiteral()->getText();
		assert(value.size() >= 2 && value.at(0) == '"' && value.at(value.size() - 1) == '"');
		value = value.substr(1, value.size() - 2);
		replace_str(value, "\\n", "\n");
		replace_str(value, "\\t", "\t");
		return (AstNode*)new AstNodeLiteral(value);
	} else {
		panicf("unknown literal");
	}
	return nullptr;
}
std::any Visitor::visitExpr_primary_identifier(PinlangParser::Expr_primary_identifierContext* ctx) {
	return (AstNode*)new AstNodeIdentifier(ctx->getText());
}
std::any Visitor::visitExpr_primary_parens(PinlangParser::Expr_primary_parensContext* ctx) {
	return ctx->expr()->accept(this);
}
std::any Visitor::visitExpr_relational(PinlangParser::Expr_relationalContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string restriction_name;
	std::string op;
	if (ctx->MUL() != NULL) {
		op = "mul";
	} else if (ctx->DIV() != NULL) {
		op = "div";
	} else if (ctx->MOD() != NULL) {
		op = "mod";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, restriction_name, op, right);
}
std::any Visitor::visitExpr_logical(PinlangParser::Expr_logicalContext* ctx) {
	panicf("not implemented yet\n");
	return nullptr;
}
std::any Visitor::visitExpr_additive(PinlangParser::Expr_additiveContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string restriction_name;
	std::string op;
	if (ctx->ADD() != nullptr) {
		op = "add";
	} else if (ctx->SUB() != nullptr) {
		op = "sub";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, restriction_name, op, right);
}
std::any Visitor::visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* ctx) {
	return ctx->expr_primary()->accept(this);
}
/*
 * 解析一个statement, 返回AstNode*
 */
std::any Visitor::visitStatement(PinlangParser::StatementContext* ctx) {
	if (ctx->expr() != nullptr)
		return ctx->expr()->accept(this);
	else if (ctx->stmt_vardef() != nullptr)
		return ctx->stmt_vardef()->accept(this);
	else if (ctx->stmt_fndef() != nullptr) {
		return ctx->stmt_fndef()->accept(this);
	} else if (ctx->stmt_block() != nullptr) {
		return ctx->stmt_block()->accept(this);
	} else if (ctx->stmt_return() != nullptr) {
		return ctx->stmt_return()->accept(this);
	} else if (ctx->stmt_restriction_def() != nullptr) {
		return ctx->stmt_restriction_def()->accept(this);
	} else {
		panicf("bug");
	}
}
std::any Visitor::visitLiteral(PinlangParser::LiteralContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitStart(PinlangParser::StartContext* ctx) {
	std::vector<AstNode*> stmts;
	for (auto n : ctx->statement()) {
		std::any ret = n->accept(this);
		if (!ret.has_value()) {
			panicf("visitor returns null");
		} else {
			// log_debug("visitor returns %s", ret.type().name());
		}
		stmts.push_back(std::any_cast<AstNode*>(ret));
	}
	return new AstNodeBlockStmt(stmts);
}
std::any Visitor::visitStmt_vardef(PinlangParser::Stmt_vardefContext* ctx) {
	std::string var_name	 = ctx->Identifier()->getText();
	TypeId		declared_tid = TYPE_ID_INFER;
	if (ctx->type() != NULL) {
		declared_tid = std::any_cast<TypeId>(ctx->type()->accept(this));
	}
	AstNode* init_expr = NULL;
	if (ctx->expr() != NULL) {
		init_expr = std::any_cast<AstNode*>(ctx->expr()->accept(this));
	}

	bool is_const = ctx->CONST() != nullptr;

	return (AstNode*)new AstNodeVarDef(var_name, declared_tid, init_expr, is_const);
}
struct ParserParameter {
	std::string name;
	TypeId		tid;
};
/*
 * 解析参数
 * @return 返回ParserParameter
 */
std::any Visitor::visitParameter(PinlangParser::ParameterContext* ctx) {
	ParserParameter param;
	if (ctx->Identifier() != nullptr) {
		param.name = ctx->Identifier()->getText();
	}
	param.tid = std::any_cast<TypeId>(ctx->type()->accept(this));
	return param;
}
/*
 * 解析一组参数
 * @return std::vector<ParserParameter>
 */
std::any Visitor::visitParameter_list(PinlangParser::Parameter_listContext* ctx) {
	std::vector<ParserParameter> params;
	for (auto iter : ctx->parameter()) {
		params.push_back(std::any_cast<ParserParameter>(iter->accept(this)));
	}
	return params;
}
/*
 * 解析一个函数定义
 * @return
 */
std::any Visitor::visitStmt_fndef(PinlangParser::Stmt_fndefContext* ctx) {
	std::string					 fn_name	= ctx->Identifier()->getText();
	std::vector<ParserParameter> params		= std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	AstNodeBlockStmt*			 body		= dynamic_cast<AstNodeBlockStmt*>(std::any_cast<AstNode*>(ctx->stmt_block()->accept(this)));
	TypeId						 return_tid = TYPE_ID_NONE;
	if (ctx->type() != nullptr) {
		return_tid = std::any_cast<TypeId>(ctx->type()->accept(this));
	}

	std::vector<Parameter> params_type;
	for (auto iter : params) {
		params_type.push_back({.arg_tid = iter.tid});
	}
	TypeId fn_tid = g_typemgr.GetOrAddTypeFn(params_type, return_tid);

	std::vector<std::string> params_name;
	for (auto iter : params) {
		params_name.push_back(iter.name);
	}
	return (AstNode*)new AstNodeFnDef(fn_tid, fn_name, params_name, body);
}
std::any Visitor::visitStmt_block(PinlangParser::Stmt_blockContext* ctx) {
	std::vector<AstNode*> stmts;
	for (auto iter : ctx->statement()) {
		stmts.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}
	return (AstNode*)new AstNodeBlockStmt(stmts);
}
std::any Visitor::visitStmt_return(PinlangParser::Stmt_returnContext* ctx) {
	AstNode* expr = nullptr;
	if (ctx->expr() != nullptr) {
		expr = std::any_cast<AstNode*>(ctx->expr()->accept(this));
	}
	return (AstNode*)new AstNodeReturn(expr);
}
/*
 * 解析expr列表
 * 返回 std::vector<AstNode*>
 */
std::any Visitor::visitExpr_list(PinlangParser::Expr_listContext* ctx) {
	std::vector<AstNode*> list;
	for (auto iter : ctx->expr()) {
		list.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}
	return list;
}
std::any Visitor::visitExpr_primary_fncall(PinlangParser::Expr_primary_fncallContext* ctx) {
	AstNode* fn_expr = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));

	std::vector<AstNode*> list = std::any_cast<std::vector<AstNode*>>(ctx->expr_list()->accept(this));

	return (AstNode*)new AstNodeFnCall(fn_expr, list);
}

struct ParserFnDeclare {
	std::string					 fnname;
	std::vector<ParserParameter> param_list;
	TypeId						 ret_tid;
	TypeId						 fn_tid;
};
/*
 * 解析函数声明. 返回 ParserFnDeclare
 */
std::any Visitor::visitStmt_fn_declare(PinlangParser::Stmt_fn_declareContext* ctx) {
	std::string					 fnname			   = ctx->Identifier()->getText();
	std::vector<ParserParameter> parser_param_list = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	TypeId						 ret_tid		   = TYPE_ID_NONE;
	if (ctx->type() != nullptr) {
		ret_tid = std::any_cast<TypeId>(ctx->type()->accept(this));
	}

	std::vector<Parameter> params_type;
	for (auto iter : parser_param_list) {
		params_type.push_back({.arg_tid = iter.tid});
	}
	TypeId fn_tid = g_typemgr.GetOrAddTypeFn(params_type, ret_tid);

	return ParserFnDeclare{
		.fnname		= fnname,
		.param_list = parser_param_list,
		.ret_tid	= ret_tid,
		.fn_tid		= fn_tid,
	};
}
std::any Visitor::visitStmt_restriction_def(PinlangParser::Stmt_restriction_defContext* ctx) {
	std::string							   name = ctx->Identifier()->getText();
	std::vector<TypeInfoRestriction::Rule> rules;
	for (auto iter : ctx->stmt_fn_declare()) {
		ParserFnDeclare fn_declare = std::any_cast<ParserFnDeclare>(iter->accept(this));
		rules.push_back(TypeInfoRestriction::Rule{
			.fn_name = fn_declare.fnname,
			.fn_tid	 = fn_declare.fn_tid,
		});
	}

	TypeId tid = g_typemgr.GetOrAddTypeRestriction(name, rules);

	return (AstNode*)new AstNodeRestriction(tid);
}
