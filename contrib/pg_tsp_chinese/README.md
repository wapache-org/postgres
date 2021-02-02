
代码源自jieba分词, 要好好的研究研究它的代码, 彻底搞懂中文分词, 然后才能进一步优化, 提高全文检索的效果和效率

## 中文分词

目前中文分词技术主要分为两类，基于词典的分词方法，基于概率统计的分词方法。

1. 基于词典分词

   顾名思义，根据已有词典进行分词，类似于查字典。基于词典的分词方法简单、速度快，效果也还可以，但对歧义和新词的处理不是很好，对词典中未登录的词没法进行处理。主要有：

   - 正向最大匹配
   - 逆向最大匹配
   - 双向最大匹配
   - 最少分词数分词

2. 基于概率统计分词
   
   基于统计的分词方法是从大量已经分词的文本中，利用统计学习方法来学习词的切分规律，从而实现对未知文本的切分。
   简单的来说就是从已有的大量文本中训练模型，对未知文本进行预测，得到分词结果。
   主要有：
   
   - HMM
   - CRF
   - Bilstm-CRF

3. 总结

基于词典分词，更有利于根据人们意愿得到想要的分词结果，训练速度快。

基于概率统计模型分词，可以处理未知文本，但需要大量的训练文本才能保证效果。

## jieba分词

jieba分词原理

jieba分词结合了词典和统计学习方法，分词效果快，也能处理未知文本。
词典部分是作者找的语料dict.txt(安装jieba后可以在site-packages/目录下找到，大约5M大小)，
统计学习模型部分是一个训练好的HMM模型。

分词过程如下：

 - 根据默认词典进行前缀词典初始化
 - 基于前缀词典实现高效的词图扫描，生成句子中汉字所有可能成词情况所构成的有向无环图
 - 采用了动态规划查找最大概率路径, 找出基于词频的最大切分组合
 - 对于未登录词，采用了基于汉字成词能力的 HMM 模型。

总的来说，
jieba分词主要是基于统计词典，构造一个前缀词典；
然后利用前缀词典对输入句子进行切分，得到所有的切分可能，根据切分位置，构造一个有向无环图；
通过动态规划算法，计算得到最大概率路径，也就得到了最终的切分形式。

### 特点：

支持三种分词模式：

 - 精确模式，试图将句子最精确地切开，适合文本分析；
 - 全模式，把句子中所有的可以成词的词语都扫描出来, 速度非常快，但是不能解决歧义；
 - 搜索引擎模式，在精确模式的基础上，对长词再次切分，提高召回率，适合用于搜索引擎分词。

支持繁体分词

支持自定义词典

### 主要算法：

基于前缀词典实现词图扫描，生成句子中汉字所有可能成词情况所构成的有向无环图（DAG），采用动态规划查找最大概率路径，找出基于词频的最大切分组合；
对于未登录词，采用了基于汉字成词能力的 HMM模型，采用Viterbi算法进行计算；
基于Viterbi算法的词性标注；
分别基于tfidf和textrank模型抽取关键词

jieba切词默认是精确模式，先利用基于词典的算法得到分词，对于未登录词利用hmm算法进行拆分。
而对于全模式，直接将jieba切词过程中产生的dag图处理后返回。
所以可见，要对分词进行优化，主要包括两个方面：

 - 丰富词库，常用方法有引入新词库，新词发现系列等
 - 优化HMM或者用更强的模型入CRF、LSTM等替代HMM

```python

# 模式一 精确模式：
import jieba
s = "我们都是好孩子"

jieba.cut(s)
# re: 我们 都 是 好孩子
# https://img-blog.csdn.net/20180709104223992
# 先简单实用正则表达式最一下拆分，然后根据cut_all, HMM的选项，用不同的方法切词，其中HMM默认是打开的。

# 通过预训练得到HMM的初始状态、转移矩阵、发射矩阵，然后利用Viterbi算法进行动态规划，得到词性标注，进而达到切词的目的。


# 模式二 全模式：
jieba.cut(s, cut_all=True)
# re: 我们 都 是 好孩子 孩子

# 该模式最简单就是把构成的DAG图，返回给前端。
# 先利用基于词典的分词方法，通过动态规划求最优分割点，然后对于未登陆词，利用HMM进行切词处理。
# 

# 模式三 不适用HMM：
jieba.cut(s, HMM = False)


```


```python
    # 基于词典在带权重的DAG图上进行动态规划，其中每个节点的权重为词频
    def calc(self, sentence, DAG, route):
        N = len(sentence)
        route[N] = (0, 0)
        logtotal = log(self.total)
        for idx in xrange(N - 1, -1, -1):
            route[idx] = max((log(self.FREQ.get(sentence[idx:x + 1]) or 1) -
                              logtotal + route[x + 1][0], x) for x in DAG[idx])
```

对于结点Wi和其可能存在的多个后继Wj和Wk，有:
任意通过Wi到达Wj的路径的权重为该路径通过Wi的路径权重加上Wj的权重{Ri->j} = {Ri + weight(j)} ；
任意通过Wi到达Wk的路径的权重为该路径通过Wi的路径权重加上Wk的权重{Ri->k} = {Ri + weight(k)} ；
对于整个句子的最优路径Rmax和一个末端节点Wx，对于其可能存在的多个前驱Wi，Wj，Wk…,设到达Wi，Wj，Wk的最大路径分别为Rmaxi，Rmaxj，Rmaxk，有：
Rmax = max(Rmaxi,Rmaxj,Rmaxk…) + weight(Wx)
于是问题转化为：
求Rmaxi, Rmaxj, Rmaxk…
组成了最优子结构，子结构里面的最优解是全局的最优解的一部分。
很容易写出其状态转移方程：
Rmax = max{(Rmaxi,Rmaxj,Rmaxk…) + weight(Wx)} 这是一个自底向下的动态规划问题

## 代码说明

### 插件相关:

- jieba.h
- jieba.cpp
- pg_jieba.c
- pg_jieba.control
- pg_jieba.sql
- pg_jieba--unpackaged.sql

### 解析相关:

- Jieba.hpp
  - LocWord
- DictTrie.hpp  : 存放词典的数据结构
  - UserWordWeightOption
  - DictTrie
- SegmentBase.hpp : 分词逻辑实现
  - SegmentTagged.hpp
  - FullSegment.hpp
  - HMMSegment.hpp : Hidden Markov Model, 隐式马尔科夫模型
  - MPSegment.hpp  : Max Probability, 最大概率法
  - MixSegment.hpp : 混合MPSegment和HMMSegment两者同时使用
  - QuerySegment.hpp
- HMMModel.hpp  
- PosTagger.hpp  : 从词典里获取某个字符串对应的tag
- PreFilter.hpp  
  - PreFilter
  - Range
- KeywordExtractor.hpp : 使用经典的TF-IDF算法, 需要用到idf.utf8词典提供IDF(Inverse Document Frequency)信息
- TextRankExtractor.hpp
  - TextRankExtractor : 计算排名用的
  - WordGraph
- Trie.hpp  
  - DictUnit
  - Dag
  - TrieNode
  - Trie
- Unicode.hpp
  - Word
  - RuneStr
  - Rune:uint32_t
  - WordRange
  - RuneStrLite
  - RuneStrArray:limonp::LocalVector<struct RuneStr>
  - Unicode:limonp::LocalVector<Rune>

### 工具相关:

- ArgvContext.hpp  : 用来解析命令行参数的
  - ArgvContext
- NonCopyable.hpp : 用于标记类实例是不可复制的
- BlockingQueue.hpp  : 阻塞队列
- BoundedBlockingQueue.hpp  : 有大小限制的阻塞队列
- BoundedQueue.hpp    : 有大小限制的队列
- Closure.hpp  : 闭包类?
  - ClosureInterface
  - Closure0
  - Closure1
  - Closure2
  - Closure3
  - ObjClosure0
  - ObjClosure1
  - ObjClosure2
  - ObjClosure3
- Colors.hpp  
  - Color
- Condition.hpp  
- Config.hpp  
- FileLock.hpp  
- ForcePublic.hpp  
- LocalVector.hpp  
- Logging.hpp  
  - Logger
- Md5.hpp  
- MutexLock.hpp  
- StdExtension.hpp  
- StringUtil.hpp  
- Thread.hpp  
  - IThread
- ThreadPool.hpp
  - Worker
  - ThreadPool

### 词典文件

略


