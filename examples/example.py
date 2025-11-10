#!/usr/bin/env python3
"""
Example usage of the array_sorter library
"""

import random

import array_sorter

def main():
    print("Array Sorter Library - Example Usage")
    print("=" * 50)
    
    # Example 1: Sort a simple array
    print("\n1. Sorting a simple array:")
    numbers = [3.5, 1.2, 4.7, 2.1, 5.9, 0.5]
    print(f"   Before: {numbers}")
    array_sorter.sort(numbers)
    print(f"   After:  {numbers}")
    
    # Example 2: Sort a random array
    print("\n2. Sorting a random array:")
    random.seed(42)
    random_array = [random.uniform(-100, 100) for _ in range(10)]
    print(f"   Before: {random_array}")
    array_sorter.sort(random_array)
    print(f"   After:  {random_array}")
    
    # Example 3: Sort with duplicates
    print("\n3. Sorting an array with duplicates:")
    duplicates = [3.0, 1.0, 3.0, 2.0, 1.0, 2.0]
    print(f"   Before: {duplicates}")
    array_sorter.sort(duplicates)
    print(f"   After:  {duplicates}")
    
    # Example 4: Sort negative numbers
    print("\n4. Sorting negative numbers:")
    negatives = [-5.0, 3.0, -1.0, 0.0, 2.0, -10.0]
    print(f"   Before: {negatives}")
    array_sorter.sort(negatives)
    print(f"   After:  {negatives}")

    # Example 5: Reverse sorting
    print("\n5. Sorting in reverse order:")
    data = [1.0, 4.0, 2.5, 0.0]
    print(f"   Before: {data}")
    array_sorter.sort(data, reverse=True)
    print(f"   After:  {data}")

    # Example 6: Getting a sorted copy without mutating the original
    print("\n6. Sorted copy (non-mutating):")
    frozen = (9.0, -1.0, 3.0)
    sorted_copy = array_sorter.sorted(frozen)
    print(f"   Original: {frozen}")
    print(f"   Sorted:   {sorted_copy}")
    
    print("\n" + "=" * 50)
    print("All examples completed successfully!")

if __name__ == "__main__":
    main()
