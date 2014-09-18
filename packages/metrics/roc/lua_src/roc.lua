metrics = metrics or {}
--

local function check_matrix(m)
  assert(class.is_a(m,matrix), "Needs a matrix as argument")
  local dim = m:dim()
  assert( #dim == 1 or
            (#dim == 2 and
               (dim[1] == 1 or dim[2] == 1)),
          "Needs a row or column vector" )
end

--

local roc,roc_methods = class("metrics.roc")
metrics.roc = roc -- global environment

april_set_doc(roc,
              {
                class="class",
                summary="ROC curve class, for one-class problems",
})

roc.constructor =
  april_doc{
    class = "method",
    summary = "Constructor without arguments",
  } ..
  april_doc{
    class = "method",
    summary = "Constructor with matrix arguments",
    params = {
      { "A matrix with classifier output probabilities in natural scale", },
      { "A matrix with target class: 0 or 1", },
    },
  } ..
  function(self,outputs,targets)
    self.data = {}
    self.P = 0
    self.N = 0
    if outputs or targets then self:add(outputs,targets) end
  end

roc_methods.add =
  april_doc{
    class = "method",
    summary = "Adds an output and target class for ROC computation",
    params = {
      { "A matrix with classifier output probabilities in natural scale", },
      { "A matrix with target class: 0 or 1", },
    },
  } ..
  function(self,outputs,targets)
    check_matrix(outputs)
    check_matrix(targets)
    local data,P,N = self.data,0,0
    outputs:map(targets, function(x,y)
                  data[#data+1] = {x,y}
                  if y > 0 then P=P+1 else N=N+1 end
    end) -- output,target
    self.P = self.P + P
    self.N = self.N + N
  end

roc_methods.compute_curve =
  april_doc{
    class = "method",
    summary = "Computes the ROC curve with all added data",
    outputs = {
      {"A matrix with N rows and 3 columns, the first column is FPR, second",
       "is TPR and the last is the value <= threshold"},
    },
  } ..
  function(self)
    local data = self.data
    local P = self.P
    local N = self.N
    table.sort(data, function(a,b) return a[1]<b[1] end)
    local result = { }
    local TP,FP = P,N
    for i=1,#data do
      local TPR,FPR = TP/P,FP/N
      result[#result+1] = { FPR, TPR, data[i][1] }
      if data[i][2] > 0 then
        TP = TP - 1
      else
        FP = FP - 1
      end
    end
    result[#result+1] = { 0, 0, 1 }
    table.sort(result,
               function(a,b)
                 if a[1] < b[1] then return true
                 elseif a[1] > b[1] then return false
                 else return a[2] < b[2]
                 end
    end)
    local out = matrix(#result,3)
    for i,v in ipairs(result) do
      out:set(i,1,v[1]):set(i,2,v[2]):set(i,3,v[3])
    end
    return out
  end

roc_methods.compute_area =
  april_doc{
    class="method",
    summary="Computes the Area Under the Curve",
    outputs={ "The area" },
  } ..
  function(self)
    local cv = self:compute_curve()
    local area = 0
    for i = 2,cv:dim(1) do
      area = area +
        (cv:get(i,1) - cv:get(i-1,1)) *
        (cv:get(i,2) + cv:get(i-1,2))*0.5
    end
    return area
  end

roc_methods.reset =
  april_doc{
    class="method",
    summary="Resets all the intermediate data",
  } ..
  function(self)
    self.data = {}
    self.P    = 0
    self.N    = 0
    collectgarbage("collect")
  end

return roc
