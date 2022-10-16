#include "source_info.h"

#include "antlr4-common.h"
#include "antlr4-runtime.h"

void SourceInfo::Init(antlr4::Token* token) {
	m_source_filepath = token->getTokenSource()->getSourceName();
	m_line_num		  = token->getTokenSource()->getLine()-1;
	m_start_index	  = token->getCharPositionInLine();
	m_stop_index	  = m_start_index + token->getStopIndex() - token->getStartIndex();
}
std::string SourceInfo::ToString(){
	char buf[32];
	snprintf(buf, sizeof(buf), "%s:%d:%d-%d", m_source_filepath.c_str(), m_line_num, m_start_index, m_stop_index);
	return std::string(buf);
}
