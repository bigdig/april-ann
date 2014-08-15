aprilann = { _NAME = "APRIL-ANN" }

make_deprecated_function = function(name, new_name, new_func)
  return function(...)
    if new_name and new_func then
      io.stderr:write(debug.traceback(string.format("Warning: %s is in deprecated state, use %s instead",
                                                    name, new_name)))
      io.stderr:write("\n")
      return new_func(...)
    else
      error(string.format("%s is in deprecated state", name))
    end
  end
end