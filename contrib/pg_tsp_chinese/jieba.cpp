/*-------------------------------------------------------------------------
 *
 * jieba.cpp
 *	  Contains functions which acted as an interface between the C++ and C
 *
 * Author: Jaimin Pan <jaimin.pan@gmail.com>
 *
 * IDENTIFICATION
 *	  jieba.cpp
 *
 *-------------------------------------------------------------------------
 */


#include <string>
#include <vector>

#include "jieba.h"

#include "parser/Jieba.hpp"

using namespace cppjieba;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

struct JiebaCtx
{
    /** 存放内置词典和用户词典的数据结构 */
	DictTrie* dict_trie_;
    /** 使用了HMM的模式才用到 */
	HMMModel* hmm_model_;

    /** 对应 MODE_MIX 模式 */
	// They share the same dict trie and model
	MixSegment* mix_seg_;
    /** 对应 MODE_QRY 模式 */
	QuerySegment* query_seg_;
    /** 对应 MODE_MP 模式 */
	MPSegment* mp_seg_;
    /** 对应 MODE_HMM 模式 */
	HMMSegment* hmm_seg_;

    /** map<string, int> */
	unordered_map<string, int> lex_id_;
};

struct ParStat
{
    /** words属性的iter */
	vector<string>::iterator iter;
    /** 分词后的词的列表 */
	vector<string> *words;
    /** iter--指向的word的信息 */
	JiebaResult cur;
};

static int Jieba_LookupType(JiebaCtx* ctx, const string& str);

JiebaCtx *
Jieba_New(const char* dict_path, const char* model_path, const char* user_dict_path)
{
	JiebaCtx* ctx = new JiebaCtx();
	ctx->dict_trie_ = new DictTrie(dict_path, user_dict_path);
	ctx->hmm_model_ = new HMMModel(model_path);
	ctx->mix_seg_ = new MixSegment(ctx->dict_trie_, ctx->hmm_model_);
	ctx->query_seg_ = new QuerySegment(ctx->dict_trie_, ctx->hmm_model_);
	ctx->mp_seg_ = new MPSegment(ctx->dict_trie_);
	ctx->hmm_seg_ = new HMMSegment(ctx->hmm_model_);

	int num_types = sizeof(lex_descr) / sizeof(lex_descr[0]);
	for (auto i = 1; i < num_types; ++i) {
		ctx->lex_id_.insert({lex_descr[i].token, i});
	}

	return ctx;
}

void
Jieba_Free(JiebaCtx* ctx)
{
	delete ctx->mix_seg_;
	delete ctx->query_seg_;
	delete ctx->mp_seg_;
	delete ctx->hmm_seg_;

	delete ctx->dict_trie_;
	delete ctx->hmm_model_;

	delete ctx;
}

/**
 * 分词?
 *
 *
 * @param ctx
 * @param str 原始字符串
 * @param len 原始字符串长度
 * @param mode 分词模式
 * @return
 */
ParStat *
Jieba_Cut(JiebaCtx* ctx, const char* str, int len, int mode)
{
	SegmentBase* x;

	string sentence(str, len);
	vector<string> *words = new vector<string>();

	switch (mode) {
		case MODE_MIX:
			x = (SegmentBase*) ctx->mix_seg_;
			break;
		case MODE_MP:
			x = (SegmentBase*) ctx->mp_seg_;
			break;
		case MODE_HMM:
			x = (SegmentBase*) ctx->hmm_seg_;
			break;
		case MODE_QRY:
			x = (SegmentBase*) ctx->query_seg_;
			break;
		default:
			x = (SegmentBase*) ctx->mix_seg_;
			break;
	}

	x->Cut(sentence, *words);

	ParStat* stat = ParStat_New();
	stat->words = words;
	stat->iter = words->begin();

	return stat;
}

/**
 * 获取下一个分词
 *
 * @param ctx
 * @param stat
 * @return
 */
JiebaResult *
Jieba_GetNext(JiebaCtx* ctx, ParStat* stat)
{
	if (!stat->words)
		return NULL;

	vector<string>::iterator& cur_iter = stat->iter;
	vector<string>::iterator end_iter = stat->words->end();

	JiebaResult* result = &(stat->cur);

	if (cur_iter == end_iter)
		return NULL;

	result->str = (char *) cur_iter->c_str();
	result->len = cur_iter->length();
	result->attr = Jieba_LookupType(ctx, *cur_iter);

	++cur_iter;

	return result;
}

/**
 * 应该是词性???
 *
 * @param ctx
 * @param str 来自`*ParStat.iter`
 * @return
 */
static int
Jieba_LookupType(JiebaCtx* ctx, const string& str) {
	MixSegment* x = (MixSegment*) ctx->mix_seg_;

	string typ = x->LookupTag(str);

	unordered_map<string, int>::const_iterator got = ctx->lex_id_.find(typ);

	if (got == ctx->lex_id_.end())
		return 0;

	return got->second;
}

ParStat *
ParStat_New()
{
	ParStat* stat = new ParStat();

	return stat;
}

void
ParStat_Free(ParStat* stat)
{
	if (stat->words)
		delete stat->words;
	delete stat;
}

#ifdef __cplusplus
}
#endif

