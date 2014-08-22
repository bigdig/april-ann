/*
 * This file is part of APRIL-ANN toolkit (A
 * Pattern Recognizer In Lua with Artificial Neural Networks).
 *
 * Copyright 2014, Salvador España-Boquera, Adrian Palacios Corella, Francisco
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
#ifndef FEATURE_BASED_LM_H
#define FEATURE_BASED_LM_H

#include <stdint.h>
#include "april_assert.h"
#include "bunch_hashed_LM.h"
#include "error_print.h"
#include "function_interface.h"
#include "history_based_LM.h"
#include "logbase.h"
#include "token_vector.h"
#include "trie_vector.h"
#include "unused_variable.h"
#include "vector.h"

namespace LanguageModels {

  template <typename Key, typename Score>
  class FeatureBasedLM;

  template <typename Key, typename Score>
  class FeatureBasedLMInterface : public HistoryBasedLMInterface <Key,Score>, public BunchHashedLMInterface <Key, Score> {
    friend class FeatureBasedLM<Key,Score>;

    Functions::FunctionInterface *filter;

  protected:
    FeatureBasedLMInterface(FeatureBasedLM<Key,Score>* model) :
      HistoryBasedLMInterface<Key,Score>(model),
      BunchHashedLMInterface<Key,Score>(model) {
      filter = model->getFilter();
      IncRef(filter);
    }

    typedef typename BunchHashedLMInterface<Key,Score>::KeyWordHash KeyWordHash;
    typedef typename BunchHashedLMInterface<Key,Score>::WordResultHash WordResultHash;
    typedef typename BunchHashedLMInterface<Key,Score>::KeyScoreMultipleBurdenTuple KeyScoreMultipleBurdenTuple;

    virtual april_utils::vector<Score> &executeQueries(basics::Token *ctxts, basics::Token *words) = 0;

    virtual void computeKeysAndScores(KeyWordHash &ctxt_hash,
                                      unsigned int bunch_size) {
      april_assert(sizeof(WordType) == sizeof(uint32_t));
      basics::TokenBunchVector *bunch_of_tokens = new basics::TokenBunchVector();
      basics::TokenVectorUint32 *word_tokens = new basics::TokenVectorUint32();

      // For each context key entry
      for (typename KeyWordHash::iterator it = ctxt_hash.begin();
        it != ctxt_hash.end(); ++it) {
        Key context_key = it->first;
        WordResultHash &word_hash = it->second;
        // offset init'd to 0
        unsigned int offset = 0;
        basics::TokenVectorUint32 *context_tokens = new basics::TokenVectorUint32();
        WordType *context_words = new WordType[this->HistoryBasedLMInterface<Key,Score>::model->ngramOrder()-1];
        const unsigned int context_size = this->getContextProperties(context_key,
                                                                     context_words,
                                                                     offset);

        for (unsigned int i = 0; i < context_size; i++)
          context_tokens->push_back(context_words[i]);

        // For each word entry
        for (typename WordResultHash::iterator it2 = word_hash.begin();
          it2 != word_hash.end(); ++it2) {
          WordType word = it2->first;
          KeyScoreMultipleBurdenTuple &result_tuple = it2->second;

          // First pass we get the next key
          // collect context and word tokens
          result_tuple.key_score.key = HistoryBasedLMInterface<Key,Score>::getDestinationKey(context_words,
                                                                    offset,
                                                                    context_size,
                                                                    word);
          bunch_of_tokens->push_back(context_tokens);
          word_tokens->push_back(word);
        }
      }

      unsigned int total_tokens = bunch_of_tokens->size();
      basics::TokenBunchVector *tmp_bunch_tokens = new basics::TokenBunchVector();
      basics::TokenVectorUint32 *tmp_word_tokens = new basics::TokenVectorUint32();
      april_utils::vector<Score> scores;

      for (unsigned int i = 0; i < total_tokens; i += bunch_size) {
        tmp_bunch_tokens->clear();
        tmp_word_tokens->clear();
        for (unsigned int b = 0; b < bunch_size; b++) {
          unsigned int shift = i + b;
          if (shift < total_tokens) {
            tmp_bunch_tokens->push_back(bunch_of_tokens[i+b]);
            tmp_word_tokens->push_back(word_tokens[i+b]);
          } else {
            break;
          }
        }
        basics::Token *filtered_input = filter->calculate(tmp_bunch_tokens);
        april_utils::vector<Score> &tmp_scores = executeQueries(filtered_input, tmp_word_tokens);
        scores.insert(scores.end(), tmp_scores.begin(), tmp_scores.end());
      }

      unsigned int k = 0;
      for (typename KeyWordHash::iterator it = ctxt_hash.begin();
        it != ctxt_hash.end(); ++it) {
        Key context_key = it->first;
        WordResultHash &word_hash = it->second;

        for (typename WordResultHash::iterator it2 = word_hash.begin();
          it2 != word_hash.end(); ++it2) {
          KeyScoreMultipleBurdenTuple &result_tuple = it2->second;

          // Second pass to store scores at table
          result_tuple.key_score.score = scores[k++];
        }
      }
    }

  public:
    virtual ~FeatureBasedLMInterface() {
      DecRef(filter);
    }

    void incRef() {
      HistoryBasedLMInterface<Key,Score>::incRef();
      BunchHashedLMInterface<Key,Score>::incRef();
    }

    bool decRef() {
      HistoryBasedLMInterface<Key,Score>::decRef();
      BunchHashedLMInterface<Key,Score>::decRef();
      return (HistoryBasedLMInterface<Key,Score>::getRef() <= 0);
    }

    basics::Token* applyFilter(basics::Token* token) {
      return filter->calculate(token);
    }
  };

  template <typename Key, typename Score>
  class FeatureBasedLM : public HistoryBasedLM <Key,Score>, public BunchHashedLM <Key,Score> {
  private:
    Functions::FunctionInterface *filter;

  public:
    FeatureBasedLM(int ngram_order,
                   WordType init_word,
                   april_utils::TrieVector *trie_vector,
                   unsigned int bunch_size,
                   Functions::FunctionInterface *filter) :
      HistoryBasedLM<Key,Score>(ngram_order,
                                init_word,
                                trie_vector),
      BunchHashedLM<Key,Score>(bunch_size),
      filter(filter) {
      IncRef(filter);
    }

    virtual ~FeatureBasedLM() {
      DecRef(filter);
    }

    Functions::FunctionInterface* getFilter() {
      return filter;
    }

    void incRef() {
      HistoryBasedLM<Key,Score>::incRef();
      BunchHashedLM<Key,Score>::incRef();
    }

    bool decRef() {
      HistoryBasedLM<Key,Score>::decRef();
      BunchHashedLM<Key,Score>::decRef();
      return (HistoryBasedLM<Key,Score>::getRef() <= 0);
    }
  };

  typedef FeatureBasedLMInterface<uint32_t, april_utils::log_float>
  FeatureBasedLMInterfaceUInt32LogFloat;
  typedef FeatureBasedLM<uint32_t, april_utils::log_float>
  FeatureBasedLMUInt32LogFloat;
}; // closes namespace

#endif // FEATURE_BASED_LM_H
