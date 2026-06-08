#pragma once

class Heap;
class Object;

Object* MakeQuoteForm(Heap& heap);
Object* MakeAndForm(Heap& heap);
Object* MakeOrForm(Heap& heap);
Object* MakeIfForm(Heap& heap);
Object* MakeDefineForm(Heap& heap);
Object* MakeSetForm(Heap& heap);
Object* MakeLambdaForm(Heap& heap);
Object* MakeSetCarForm(Heap& heap);
Object* MakeSetCdrForm(Heap& heap);
Object* MakeBeginForm(Heap& heap);
