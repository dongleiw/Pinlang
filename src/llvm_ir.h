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

class LLVMIR {
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

	void		 SetRetValue(llvm::Value* value) { m_ret_value = value; }
	llvm::Value* GetRetValue() const { return m_ret_value; }

	void			  SetExitBlock(llvm::BasicBlock* block) { m_exit_block = block; }
	llvm::BasicBlock* GetExitBlock() const { return m_exit_block; }

private:
	llvm::LLVMContext* m_context;
	llvm::Module*	   m_module;
	llvm::IRBuilder<>* m_builder;
	//std::map<std::string, llvm::Value*> m_named_values;
	std::list<Block*> m_blocks;
	llvm::Function*	  m_cur_fn;
	llvm::Value*	  m_ret_value;	// 当前函数的返回值
	llvm::BasicBlock* m_exit_block; // 当前函数的退出block
};

#define IRC (llvm_ir.GetContext())
#define IRB (llvm_ir.GetBuilder())
#define IRM (llvm_ir.GetModule())
