
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int heapify(int arr[], int idx, int end)
{
	int largest = idx;
	int tmp = 0;

	int left = idx * 2 + 1;
	int right = idx * 2 + 2;
	
	if (left <= end && arr[largest] < arr[left]) {
		largest = left;
	}

	if (right <= end && arr[largest] < arr[right]) {
		largest = right;
	}

	if (largest != idx) {
		tmp = arr[idx];
		arr[idx] = arr[largest];
		arr[largest] = tmp;
		return heapify(arr, largest, end);
	}

	return 0;
}

int build_heap(int arr[], int num)
{
	// 从倒数第二层开始建堆，相当于从底层开始把最大数往上挪，到最后，根节点的数必然是最大数
	int i = 0;
	for (i = num/2 -1; i >= 0; i--) {
		heapify(arr, i, num-1);
	}

	return 0;
}

int sort_onheap(int arr[], int num)
{
	int i = 0;
	int tmp = 0;

	for (i = num - 1; i >= 0; i--) {
		// 将最大数置换到最后位置上
		tmp = arr[0];
		arr[0] = arr[i];
		arr[i] = tmp;
		// 重新将0开始的堆构建，这个时候第二层已经最大的数据
		heapify(arr, 0, i-1);
	}

	return 0;
}

int show_data(int arr[], int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		fprintf(stdout, "%d ", arr[i]);
	}
	fprintf(stdout, "\n");

	return 0;
}

int main(int argc, char *argv[])
{
	int arr_num = 0, i = 0;
	int *arr;

	if (argc < 3) return 1;

	arr_num = atoi(argv[1]);
	arr_num = arr_num <= argc-2 ? arr_num : argc-2;
	if (arr_num <= 0) return 1;
	arr = (int*)malloc(arr_num * sizeof(int));
	if (!arr) return 1;

	for (i = 0; i < arr_num; i++) {
		arr[i] = atoi(argv[i+2]);
	}
	
	show_data(arr, arr_num);

	build_heap(arr, arr_num);
	show_data(arr, arr_num);
	sort_onheap(arr, arr_num);

	show_data(arr, arr_num);

	if (arr) {
		free(arr);
		arr = NULL;
	}

	return 0;
}
