-- un generador de valores aleatorios... y otros parametros
bunch_size    = tonumber(arg[1]) or 64
semilla       = 1234
aleat         = random(semilla)
description   = "256 inputs 256 tanh 128 tanh 10 softmax"
inf           = -0.1
sup           =  0.1
otrorand      = random(5678)
learning_rate = 0.01
momentum      = 0.01
weight_decay  = 1e-05
max_epochs    = 10

-- training and validation
errors = {
  {2.1826949, 1.8960779},
  {1.4842647, 1.0203998},
  {0.8297270, 0.5678325},
  {0.4682161, 0.3914193},
  {0.2817744, 0.2966153},
  {0.2072810, 0.2108385},
  {0.1584013, 0.1847290},
  {0.1259473, 0.1575812},
  {0.1125821, 0.1618845},
  {0.0988079, 0.1749958},
}
epsilon = 1e-05

--------------------------------------------------------------

m1 = ImageIO.read("digits.png"):to_grayscale():invert_colors():matrix()
train_input = dataset.matrix(m1,
			     {
			       patternSize = {16,16},
			       offset      = {0,0},
			       numSteps    = {80,10},
			       stepSize    = {16,16},
			       orderStep   = {1,0}
			     })

val_input  = dataset.matrix(m1,
			    {
			      patternSize = {16,16},
			      offset      = {1280,0},
			      numSteps    = {20,10},
			      stepSize    = {16,16},
			      orderStep   = {1,0}
			    })
-- una matriz pequenya la podemos cargar directamente
m2 = matrix(10,{1,0,0,0,0,0,0,0,0,0})

-- ojito con este dataset, fijaros que usa una matriz de dim 1 y talla
-- 10 PERO avanza con valor -1 y la considera CIRCULAR en su unica
-- dimension

train_output = dataset.matrix(m2,
			      {
				patternSize = {10},
				offset      = {0},
				numSteps    = {800},
				stepSize    = {-1},
				circular    = {true}
			      })

val_output   = dataset.matrix(m2,
			      {
				patternSize = {10},
				offset      = {0},
				numSteps    = {200},
				stepSize    = {-1},
				circular    = {true}
			      })


lared = ann.mlp.all_all.generate{
  bunch_size  = bunch_size,
  topology    = description,
  random      = aleat,
  inf         = inf,
  sup         = sup,
}

-- datos para entrenar
datosentrenar = {
  input_dataset  = train_input,
  output_dataset = train_output,
  shuffle        = otrorand,
}

datosvalidar = {
  input_dataset  = val_input,
  output_dataset = val_output,
}

--lared = ann.mlp.all_all.load("jaja.net", bunch_size)
--print(lared:validate_dataset(datosvalidar))

lared:set_option("learning_rate", learning_rate)
lared:set_option("momentum",      momentum)
lared:set_option("weight_decay",  weight_decay)
lared:set_use_cuda(true, true)

lared:set_error_function(ann.error_functions.cross_entropy())

totalepocas = 0

-- ponemos esto aqui para que se inicie CUDA
errorval    = lared:validate_dataset(datosvalidar)
print("# Initial validation error:", errorval)

clock = util.stopwatch()
clock:go()

-- ann.mlp.all_all.save(lared, "wop.net", "ascii", "old")
print("Epoch Training  Validation")
for epoch = 1,max_epochs do
  collectgarbage("collect")
  -- ann.mlp.all_all.save(lared, "tmp.net", "binary", "old")
  -- lared=ann.mlp.all_all.load("tmp.net", bunch_size)
  
  totalepocas = totalepocas+1
  errortrain  = lared:train_dataset(datosentrenar)
  errorval    = lared:validate_dataset(datosvalidar)
  if math.abs(errortrain - errors[epoch][1]) > epsilon then
    error(string.format("Training error %g is not equal enough to "..
  			"reference error %g",
  			errortrain, errors[epoch][1]))
  end
  if math.abs(errorval - errors[epoch][2]) > epsilon then
    error(string.format("Validation error %g is not equal enough to "..
  			"reference error %g",
  			errorval, errors[epoch][2]))
  end
   ann.mlp.all_all.save(lared, "wop.net", "ascii", "old")
  printf("%4d  %.7f %.7f\n",
	 totalepocas,errortrain,errorval)
end

clock:stop()
cpu,wall = clock:read()
printf("Wall total time: %.3f    per epoch: %.3f\n", wall, wall/max_epochs)
printf("CPU  total time: %.3f    per epoch: %.3f\n", cpu, cpu/max_epochs)
--ann.mlp.all_all.save(lared, "red_original.net", "ascii", "old")

--for ipat,pat in datosvalidar.input_dataset:patterns() do
--  local out=lared:calculate(pat)
--  print(table.concat(out, " "))
--end

print("Test passed! OK!")
