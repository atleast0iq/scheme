#pragma once

class Heap;
class Object;

Object* MakeIsNumberFunction(Heap& heap);
Object* MakeAddFunction(Heap& heap);
Object* MakeSubtractFunction(Heap& heap);
Object* MakeMultiplyFunction(Heap& heap);
Object* MakeDivideFunction(Heap& heap);
Object* MakeMaxFunction(Heap& heap);
Object* MakeMinFunction(Heap& heap);
Object* MakeAbsFunction(Heap& heap);
Object* MakeEqualFunction(Heap& heap);
Object* MakeLessFunction(Heap& heap);
Object* MakeGreaterFunction(Heap& heap);
Object* MakeLessOrEqualFunction(Heap& heap);
Object* MakeGreaterOrEqualFunction(Heap& heap);
Object* MakeIsBooleanFunction(Heap& heap);
Object* MakeNotFunction(Heap& heap);
Object* MakeIsPairFunction(Heap& heap);
Object* MakeIsNullFunction(Heap& heap);
Object* MakeIsListFunction(Heap& heap);
Object* MakeConsFunction(Heap& heap);
Object* MakeCarFunction(Heap& heap);
Object* MakeCdrFunction(Heap& heap);
Object* MakeListFunction(Heap& heap);
Object* MakeListRefFunction(Heap& heap);
Object* MakeListTailFunction(Heap& heap);
Object* MakeIsSymbolFunction(Heap& heap);
Object* MakeModuloFunction(Heap& heap);
