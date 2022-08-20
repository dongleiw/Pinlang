#pragma once

#include "PinlangBaseVisitor.h"

class Visitor : public PinlangBaseVisitor {
public:
	virtual std::any visitType(PinlangParser::TypeContext* context) override;
	virtual std::any visitExpr_primary_literal(PinlangParser::Expr_primary_literalContext* context) override;
	virtual std::any visitExpr_primary_identifier(PinlangParser::Expr_primary_identifierContext* context) override;
	virtual std::any visitExpr_primary_parens(PinlangParser::Expr_primary_parensContext* context) override;
	virtual std::any visitExpr_relational(PinlangParser::Expr_relationalContext* context) override;
	virtual std::any visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* context) override;
	virtual std::any visitExpr_logical(PinlangParser::Expr_logicalContext* context) override;
	virtual std::any visitExpr_additive(PinlangParser::Expr_additiveContext* context) override;
	virtual std::any visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* context) override;
	virtual std::any visitStatement(PinlangParser::StatementContext* context) override;
	virtual std::any visitLiteral(PinlangParser::LiteralContext* context) override;
	virtual std::any visitStart(PinlangParser::StartContext* context) override;
	virtual std::any visitStmt_vardef(PinlangParser::Stmt_vardefContext* ctx) override;
	virtual std::any visitParameter(PinlangParser::ParameterContext* context) override;
	virtual std::any visitParameter_list(PinlangParser::Parameter_listContext* context) override;
	virtual std::any visitStmt_fndef(PinlangParser::Stmt_fndefContext* context) override;
	virtual std::any visitStmt_block(PinlangParser::Stmt_blockContext* context) override;
	virtual std::any visitStmt_return(PinlangParser::Stmt_returnContext* context) override;
	virtual std::any visitExpr_list(PinlangParser::Expr_listContext* ctx) override;
	virtual std::any visitExpr_primary_fncall(PinlangParser::Expr_primary_fncallContext *ctx)override;
};
