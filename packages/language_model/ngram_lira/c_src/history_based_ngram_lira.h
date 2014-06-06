/*
 * This file is part of APRIL-ANN toolkit (A
 * Pattern Recognizer In Lua with Artificial Neural Networks).
 *
 * Copyright 2012, Salvador España-Boquera, Adrian Palacios Corella, Francisco
 * Zamora-Martinez
 *
 * The APRIL-ANN toolkit is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef HISTORY_BASED_NGRAM_LIRA_H
#define HISTORY_BASED_NGRAM_LIRA_H

#include "ngram_lira.h"

namespace LanguageModels {

  class HistoryBasedNgramLiraLM;
  
  class HistoryBasedNgramLiraLMInterface :
    public HistoryBasedLMInterface< NgramLiraModel::Key,
                                    NgramLiraModel::Score > {
  public:
    typedef NgramLiraModel::Key Key;
    typedef NgramLiraModel::Score Score;

    Score getBestProb() const;

    Score getBestProb(const Key &k) const;
    
    Score getFinalScore(const Key &k, Score threshold);

  protected:
    friend class HistoryBasedNgramLiraLM;
    HistoryBasedNgramLiraLMInterface(HistoryBasedNgramLiraLM *model,
				     NgramLiraModel *lira_model);
    
  private:

    NgramLiraInterface *lira_interface;

    Score privateGet(const Key &key,
                     WordType word,
                     WordType *context_words,
                     unsigned int context_size);
  };

  class HistoryBasedNgramLiraLM : public HistoryBasedLM<NgramLiraModel::Key,
                                                        NgramLiraModel::Score > {
  public:
    typedef NgramLiraModel::Key Key;
    typedef NgramLiraModel::Score Score;

  private:
    NgramLiraModel *lira_model;

  public:
    HistoryBasedNgramLiraLM(int ngram_order,
                            WordType init_word,
                            april_utils::TrieVector *trie_vector,
                            NgramLiraModel *lira_model);

    virtual ~HistoryBasedNgramLiraLM();

    virtual LMInterface<Key,Score>* getInterface();
  };




} // closes namespace language_models

#endif // HISTORY_BASED_NGRAM_LIRA_H
