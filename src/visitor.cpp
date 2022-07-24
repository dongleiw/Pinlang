#include "visitor.h"

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_literal.h"
#include "astnode_operator.h"
#include "log.h"
#include "utils.h"

std::any Visitor::visitType(PinlangParser::TypeContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitPrimary_expr_literal(PinlangParser::Primary_expr_literalContext* ctx) {
	auto literal = ctx->literal();
	if (literal->IntegerLiteral() != NULL) {
		int value = str_to_int(literal->IntegerLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else {
		panicf("unknown literal");
	}
	return NULL;
}
std::any Visitor::visitPrimary_expr_identifier(PinlangParser::Primary_expr_identifierContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitPrimary_expr_parens(PinlangParser::Primary_expr_parensContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitExpr_relational(PinlangParser::Expr_relationalContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* ctx) {
	AstNode* left = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode* right= std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string op;
	if(ctx->MUL()!=NULL){
		op="*";
	}else if(ctx->DIV()!=NULL){
		op="/";
	}else if(ctx->MOD()!=NULL){
		op="%";
	}else{
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, op, right);
}
std::any Visitor::visitExpr_logical(PinlangParser::Expr_logicalContext* ctx) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitExpr_additive(PinlangParser::Expr_additiveContext* ctx) {
	AstNode* left = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode* right= std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string op;
	if(ctx->ADD()!=NULL){
		op="+";
	}else if(ctx->SUB()!=NULL){
		op="+";
	}else{
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
	return ctx->expr()->accept(this);
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
