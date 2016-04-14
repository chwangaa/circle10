from collections import defaultdict

def scale_row(row1, a, row2):
	"""
	row2 = a * row1
	>>> m = [1,2,3]
	>>> scale_row([5,4,2], 2, m)
	>>> m == [10, 8, 4]
	True
	"""
	for i, e in enumerate(row1):
		row2[i] = a*e


def add_row(row1, row2, row3):
	"""
	row3 = row1 + row2
	>>> m = [0,0,0]
	>>> add_row([1,2,3], [5,-1,0], m)
	>>> m == [6, 1, 3]
	True
	"""

	for i, e in enumerate(row1):
		row3[i] = e + row2[i]

def newRow(N):
	assert(type(N) == int)
	r = []
	for i in range(N):
		r.append(0)
	return r

class smatrix:
	def __init__(self, dmatrix):
		"""
		given a dense matrix, construct the sparse matrix
		>>> m = [[1,2,3], [1,2,3], [1,2,3]]
		>>> s = smatrix(m)
		>>> s.rank
		3
		"""
		matrix = defaultdict(list)
		num_rows = len(dmatrix)
		num_cols = len(dmatrix[0])
		for i, row in enumerate(dmatrix):
			for j, v in enumerate(row):
				if v == 0:
					continue
				else:
					matrix[v].append((i, j))

		self.matrix = matrix
		self.rank = len(matrix)
		self.num_rows = num_rows
		self.num_cols = num_cols

	def multiply(self, B, C):
		"""
		multiply the dense matrix A, with B, and store the result in C
		>>> m = [[1,2,3], [1,2,3], [1,2,3]]
		>>> s = smatrix(m)
		>>> B = [[1,1,1], [1,1,1], [1,1,1]]
		>>> C = [[0,0,0], [0,0,0], [0,0,0]]
		>>> s.multiply(B, C)
		>>> C == [[6,6,6], [6,6,6], [6,6,6]]
		True
		"""
		A = self.matrix
		N = len(B[0])
		for v, indices in A.iteritems():
			dic = defaultdict(list)
			for (i, j) in indices:
				dic[i].append(j)

			for i, b_rows in dic.iteritems():
				B_row = newRow(N)
				for row in b_rows:
					add_row(B_row, B[row], B_row)
				scale_row(B_row, v, B_row)
				add_row(B_row, C[i], C[i])
				# print C[i]

	def serializeToFile(self, file_name):
		row = self.num_rows;
		col = self.num_cols;
		rank = self.rank;
		matrix = self.matrix;
		with open(file_name, 'w') as fw:
			fw.write("%d %d %d\n"%(row, col, rank))

			for v, indices in matrix.iteritems():
				dic = defaultdict(list)
				for (i, j) in indices:
					dic[i].append(j)
				num_i = len(dic)
				fw.write("%f %d\n"%(v, num_i))
				for i, b_rows in dic.iteritems():
					fw.write("%d %d\n"%(i, len(b_rows)))
					for row in b_rows:
						fw.write("%d \n"%row)