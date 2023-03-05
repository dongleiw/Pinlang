#pragma once

#include "llvm-12/llvm/ADT/APFloat.h"
#include "llvm-12/llvm/ADT/STLExtras.h"
#include "llvm-12/llvm/IR/BasicBlock.h"
#include "llvm-12/llvm/IR/Constants.h"
#include "llvm-12/llvm/IR/DerivedTypes.h"
#include "llvm-12/llvm/IR/Function.h"
#include "llvm-12/llvm/IR/IRBuilder.h"
#include "llvm-12/llvm/IR/LLVMContext.h"
#include "llvm-12/llvm/IR/Module.h"
#include "llvm-12/llvm/IR/Type.h"
#include "llvm-12/llvm/IR/Verifier.h"
#include <list>
#include <llvm-12/llvm/IR/Value.h>

class CompileContext {
public:
	class Block {
	public:
		void		 AddNamedValue(std::string name, llvm::Value* value);
		bool		 HasNamedValue(std::string name) const;
		llvm::Value* GetNamedValue(std::string name);

	private:
		std::map<std::string, llvm::Value*> m_named_values;
	};

public:
	CompileContext();

	void Init();

	llvm::LLVMContext& GetContext() { return *m_context; }
	llvm::IRBuilder<>& GetBuilder() { return *m_builder; }
	llvm::Module&	   GetModule() { return *m_module; }

	void		 AddNamedValue(std::string name, llvm::Value* value);
	bool		 HasNamedValue(std::string name) const;
	llvm::Value* GetNamedValue(std::string name);

	void EnterBlock();
	void LeaveBlock();

	void			SetCurFn(llvm::Function* fn) { m_cur_fn = fn; }
	llvm::Function* GetCurFn() const { return m_cur_fn; }

	void CompileBegin();
	void CompileEnd();

	bool IsLeftValue() const;

	void			  SetContinueBlock(llvm::BasicBlock* bb) { m_continue_block = bb; }
	void			  SetBreakBlock(llvm::BasicBlock* bb) { m_break_block = bb; }
	llvm::BasicBlock* GetContinueBlock() const { return m_continue_block; }
	llvm::BasicBlock* GetBreakBlock() const { return m_break_block; }

private:
	llvm::LLVMContext* m_context;
	llvm::Module*	   m_module;
	llvm::IRBuilder<>* m_builder;
	std::list<Block*>  m_blocks;
	llvm::Function*	   m_cur_fn;

	// continue/break语句跳转的目标block
	// 由parent-astnode设置 (for, while, switch)
	llvm::BasicBlock* m_continue_block;
	llvm::BasicBlock* m_break_block;
};

#define IRC (cctx.GetContext())
#define IRB (cctx.GetBuilder())
#define IRM (cctx.GetModule())
