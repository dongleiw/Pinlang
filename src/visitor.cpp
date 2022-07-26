#include "visitor.h"

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_identifier.h"
#include "astnode_literal.h"
#include "astnode_operator.h"
#include "astnode_vardef.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"
#include <any>
#include <ios>

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
std::any Visitor::visitPrimary_expr_literal(PinlangParser::Primary_expr_literalContext* ctx) {
	auto literal = ctx->literal();
	if (literal->IntegerLiteral() != nullptr) {
		int value = str_to_int(literal->IntegerLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else {
		panicf("unknown literal");
	}
	return nullptr;
}
std::any Visitor::visitPrimary_expr_identifier(PinlangParser::Primary_expr_identifierContext* ctx) {
	return (AstNode*)new AstNodeIdentifier(ctx->getText());
}
std::any Visitor::visitPrimary_expr_parens(PinlangParser::Primary_expr_parensContext* ctx) {
	return ctx->expr()->accept(this);
}
std::any Visitor::visitExpr_relational(PinlangParser::Expr_relationalContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string op;
	if (ctx->MUL() != NULL) {
		op = "*";
	} else if (ctx->DIV() != NULL) {
		op = "/";
	} else if (ctx->MOD() != NULL) {
		op = "%";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, op, right);
}
std::any Visitor::visitExpr_logical(PinlangParser::Expr_logicalContext* ctx) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitExpr_additive(PinlangParser::Expr_additiveContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string op;
	if (ctx->ADD() != NULL) {
		op = "+";
	} else if (ctx->SUB() != NULL) {
		op = "+";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, op, right);
}
std::any Visitor::visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* ctx) {
	return ctx->primary_expr()->accept(this);
}
/*
 * 解析一个statement, 返回AstNode*
 */
std::any Visitor::visitStatement(PinlangParser::StatementContext* ctx) {
	if (ctx->expr() != NULL)
		return ctx->expr()->accept(this);
	else if (ctx->stmt_vardef() != NULL)
		return ctx->stmt_vardef()->accept(this);
	else
		panicf("bug");
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
			log_debug("visitor returns %s", ret.type().name());
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
