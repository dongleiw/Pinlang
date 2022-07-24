#pragma once

#include "PinlangBaseVisitor.h"

class Visitor:public PinlangBaseVisitor {
public:
	virtual std::any visitType(PinlangParser::TypeContext *context);
	virtual std::any visitPrimary_expr_literal(PinlangParser::Primary_expr_literalContext *context);
	virtual std::any visitPrimary_expr_identifier(PinlangParser::Primary_expr_identifierContext *context);
	virtual std::any visitPrimary_expr_parens(PinlangParser::Primary_expr_parensContext *context);
	virtual std::any visitExpr_relational(PinlangParser::Expr_relationalContext *context);
	virtual std::any visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext *context);
	virtual std::any visitExpr_logical(PinlangParser::Expr_logicalContext *context);
	virtual std::any visitExpr_additive(PinlangParser::Expr_additiveContext *context);
	virtual std::any visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext *context);
	virtual std::any visitStatement(PinlangParser::StatementContext *context);
	virtual std::any visitLiteral(PinlangParser::LiteralContext *context);
	virtual std::any visitStart(PinlangParser::StartContext *context);
};
