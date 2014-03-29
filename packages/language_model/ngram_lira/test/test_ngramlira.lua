local path = arg[0]:get_path()
local vocab = lexClass.load(io.open(path .. "vocab"))
local model = language_models.load(path .. "dihana3gram.lira.gz",
				  vocab, "<s>", "</s>")
local unk_id = -1
local lines_it 

print("----------------------------------------------------------------------")
print("-------------------------- USE_UNK = ALL -----------------------------")
print("----------------------------------------------------------------------")
print("\n")

lines_it = iterator(io.lines("frase")):
map( function(line) return iterator(line:gmatch("[^%s]+")) end )

for words_it in lines_it() do
  words_it = words_it:map( function(w) return (vocab:getWordId(w) or unk_id), w end )
  local sum,numwords,numunks =
    language_models.get_sentence_prob{ 
    					    lm = model,
					    words_it = words_it,
					    debug_flag = 2,
					    use_unk = "all"
					  }
end

print("----------------------------------------------------------------------")
print("------------------------ USE_UNK = CONTEXT ---------------------------")
print("----------------------------------------------------------------------")
print("\n")

lines_it = iterator(io.lines("frase")):
map( function(line) return iterator(line:gmatch("[^%s]+")) end )

for words_it in lines_it() do
  words_it = words_it:map( function(w) return (vocab:getWordId(w) or unk_id), w end )
  local sum,numwords,numunks =
    language_models.get_sentence_prob{ 
    					    lm = model,
					    words_it = words_it,
					    debug_flag = 2,
					    use_unk = "context"
					  }
end

print("----------------------------------------------------------------------")
print("------------------------- USE_UNK = NONE -----------------------------")
print("----------------------------------------------------------------------")
print("\n")

lines_it = iterator(io.lines("frase")):
map( function(line) return iterator(line:gmatch("[^%s]+")) end )

for words_it in lines_it() do
  words_it = words_it:map( function(w) return (vocab:getWordId(w) or unk_id), w end )
  local sum,numwords,numunks =
    language_models.get_sentence_prob{ 
    					    lm = model,
					    words_it = words_it,
					    debug_flag = 2,
					    use_unk = "none"
					  }
end
