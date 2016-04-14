from sparse_matrix import smatrix

def round_weights(weight_file, n, output_file):
	values = []
	with open(weight_file) as fr:
		header = fr.readline()
		for i in fr:
			values.append(round(float(i), n))

	with open(output_file, 'w') as fw:
		fw.write(header)
		for v in values:
			fw.write("%f\n"%(v))


def load_weight_matrix(weight_file, n):
	with open(weight_file) as fr:
		(k1, k2, c, row) = map(int, fr.readline().strip().split())
		assert(k1 == k2)
		col = k1 * k2 * c
		matrix = []
		for i in range(row):
			new_row = []
			for j in range(col):
				new_row.append(round(float(fr.readline()), n))
			matrix.append(new_row)
		return matrix

def load_matrix(matrix_file):
	with open(matrix_file) as fr:
		(M, N) = map(int, fr.readline().strip().split())
		matrix = []
		for i in range(M):
			new_row = []
			for j in range(N):
				new_row.append(float(fr.readline()))
			matrix.append(new_row)
		return matrix


def make_smatrix_file(dense_weight_file, output_file, n=1):
	matrix = load_weight_matrix(dense_weight_file, n)
	s = smatrix(matrix)
	s.serializeToFile(output_file)

def make_smatrix(dense_weight_file, n=1):
	matrix = load_weight_matrix(dense_weight_file, n)
	s = smatrix(matrix)
	return s