import array
import random

import pytest

import array_sorter


def test_sort_empty_list():
    """Test sorting an empty list"""
    arr = []
    array_sorter.sort(arr)
    assert arr == []


def test_sort_single_element():
    """Test sorting a single element"""
    arr = [42.0]
    array_sorter.sort(arr)
    assert arr == [42.0]


def test_sort_already_sorted():
    """Test sorting an already sorted array"""
    arr = [1.0, 2.0, 3.0, 4.0, 5.0]
    expected = arr.copy()
    array_sorter.sort(arr)
    assert arr == expected


def test_sort_reverse_sorted():
    """Test sorting a reverse sorted array"""
    arr = [5.0, 4.0, 3.0, 2.0, 1.0]
    array_sorter.sort(arr)
    assert arr == [1.0, 2.0, 3.0, 4.0, 5.0]


def test_sort_random():
    """Test sorting a random array"""
    arr = [3.5, 1.2, 4.7, 2.1, 5.9]
    array_sorter.sort(arr)
    for i in range(1, len(arr)):
        assert arr[i - 1] <= arr[i]


def test_sort_duplicates():
    """Test sorting an array with duplicate values"""
    arr = [3.0, 1.0, 3.0, 2.0, 1.0]
    array_sorter.sort(arr)
    assert arr == [1.0, 1.0, 2.0, 3.0, 3.0]


def test_sort_negative_numbers():
    """Test sorting an array with negative numbers"""
    arr = [-5.0, 3.0, -1.0, 0.0, 2.0]
    array_sorter.sort(arr)
    assert arr == [-5.0, -1.0, 0.0, 2.0, 3.0]


def test_sort_large_array():
    """Test sorting a large array"""
    size = 10000
    arr = [random.uniform(-1000.0, 1000.0) for _ in range(size)]
    array_sorter.sort(arr)
    for i in range(1, len(arr)):
        assert arr[i - 1] <= arr[i]


def test_sort_type_validation():
    """Test that non-numeric types raise appropriate errors"""
    with pytest.raises(TypeError):
        array_sorter.sort(["a", "b", "c"])


def test_sort_reverse_flag():
    """Sorting in reverse should flip the order"""
    arr = [1, 5, 3, 2]
    array_sorter.sort(arr, reverse=True)
    assert arr == [5, 3, 2, 1]


def test_sort_rejects_immutable_sequence():
    """Passing a tuple should raise because it cannot be mutated"""
    with pytest.raises(TypeError):
        array_sorter.sort((3.0, 2.0, 1.0))


def test_sort_rejects_strings():
    """Strings are sequences but should not be treated as numeric arrays"""
    with pytest.raises(TypeError):
        array_sorter.sort("abc")


def test_sort_supports_array_module():
    """array.array should be sorted in place"""
    arr = array.array("d", [5.0, -1.0, 2.0])
    array_sorter.sort(arr)
    assert arr.tolist() == [-1.0, 2.0, 5.0]


def test_sorted_copy_preserves_original():
    """array_sorter.sorted returns a new sorted list"""
    data = (4.0, 1.0, 3.0)
    result = array_sorter.sorted(data)
    assert result == [1.0, 3.0, 4.0]
    assert data == (4.0, 1.0, 3.0)


def test_sorted_copy_reverse_flag():
    """sorted(..., reverse=True) honors the direction"""
    data = [1.0, -1.0, 10.0]
    result = array_sorter.sorted(data, reverse=True)
    assert result == [10.0, 1.0, -1.0]


def test_sorted_accepts_generators():
    """Any iterable works for sorted()"""
    data = [random.uniform(-5, 5) for _ in range(5)]
    result = array_sorter.sorted((value for value in data))
    assert result == sorted(data)
