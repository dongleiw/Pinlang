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
		void		 ReplaceNamedValue(std::string name, llvm::Value* value);
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
	void		 ReplaceNamedValue(std::string name, llvm::Value* value);
	bool		 HasNamedValue(std::string name) const;
	llvm::Value* GetNamedValue(std::string name);

	void EnterBlock();
	void LeaveBlock();

	void			SetCurFn(llvm::Function* fn) { m_cur_fn = fn; }
	llvm::Function* GetCurFn() const { return m_cur_fn; }

	void CompileBegin();
	void CompileEnd();

	void SetCurFnIsMethod(bool is_method) { m_cur_fn_is_method = is_method; }
	bool CurFnIsMethod() const { return m_cur_fn_is_method; }

	void			  SetContinueBlock(llvm::BasicBlock* bb) { m_continue_block = bb; }
	void			  SetBreakBlock(llvm::BasicBlock* bb) { m_break_block = bb; }
	llvm::BasicBlock* GetContinueBlock() const { return m_continue_block; }
	llvm::BasicBlock* GetBreakBlock() const { return m_break_block; }

	void		 SetMethodObj(llvm::Value* obj) { m_method_obj = obj; }
	llvm::Value* GetMethodObj() const { return m_method_obj; }

private:
	llvm::LLVMContext* m_context;
	llvm::Module*	   m_module;
	llvm::IRBuilder<>* m_builder;
	std::list<Block*>  m_blocks;
	llvm::Function*	   m_cur_fn;
	bool			   m_cur_fn_is_method;
	llvm::Value*	   m_method_obj;

	// continue/break语句跳转的目标block
	// 由parent-astnode设置 (for, while, switch)
	llvm::BasicBlock* m_continue_block;
	llvm::BasicBlock* m_break_block;
};

class CompileResult {
public:
	CompileResult() {
		m_compile_result = nullptr;
		m_is_fn			 = false;
		m_thisobj		 = nullptr;
	}

	CompileResult& SetResult(llvm::Value* result) {
		m_compile_result = result;
		return *this;
	}
	CompileResult& SetResultFn(llvm::Function* result) {
		m_compile_result = result;
		m_is_fn			 = true;
		return *this;
	}
	CompileResult& SetResultMethod(llvm::Function* result, llvm::Value* thisobj) {
		m_compile_result = result;
		m_is_fn			 = true;
		m_thisobj		 = thisobj;
		return *this;
	}

	llvm::Value*	GetResult() const;
	llvm::Function* GetResultFn() const;
	llvm::Function* GetResultMethod() const;
	bool			IsMethod() const;
	llvm::Value*	GetThisObj() const;

private:
	llvm::Value* m_compile_result;
	bool		 m_is_fn;
	llvm::Value* m_thisobj;
};
#define IRC (cctx.GetContext())
#define IRB (cctx.GetBuilder())
#define IRM (cctx.GetModule())
