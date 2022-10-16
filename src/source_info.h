#pragma once

#include "Token.h"
class SourceInfo {
public:
	SourceInfo(){}
	void Init(antlr4::Token* token);
	std::string ToString();
private:
	std::string m_source_filepath;
	int m_line_num;
	int m_start_index;
	int m_stop_index;
};
