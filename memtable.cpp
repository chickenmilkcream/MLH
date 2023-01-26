
#include <iostream>
#include "memtable.h"
#include "AvlTree.h"
void put(uint64_t key, uint64_t val) {

}




uint64_t get(uint64_t key) {

}

void Memtable::scan(uint64_t min_key, uint64_t max_key) {

}

uint64_t Memtable::get(uint64_t key) {
    return 0;
}

void Memtable::put(uint64_t key, uint64_t val) {

}

uint64_t Memtable::get_helper(Entry root, uint64_t key) {
    return 0;
}

void Memtable::put_helper(Entry root, uint64_t key, uint64_t val) {

}

void Memtable::scan_helper(Entry root, uint64_t min_key, uint64_t max_key) {

}


int main( )
{
    const int ITEM_NOT_FOUND = -9999;
    AvlTree<int> t( ITEM_NOT_FOUND ), t2( ITEM_NOT_FOUND );
//    int NUMS = 40000;
//    const int GAP  =   37;
//    int i;
//
//    cout << "Checking... (no more output means success)" << endl;
//
//    for( i = GAP; i != 0; i = ( i + GAP ) % NUMS )
//        t.insert( i, 69);
//
//    if( NUMS < 40 )
//        t.printTree( );
//    if( t.findMin( ) != 1 || t.findMax( ) != NUMS - 1 )
//        cout << "FindMin or FindMax error!" << endl;
//
//    t2 = t;
//
//    for( i = 1; i < NUMS; i++ )
//        if( t2.find( i ) != i )
//            cout << "Find error1!" << endl;
//    if( t2.find( 0 ) != ITEM_NOT_FOUND )
//        cout << "ITEM_NOT_FOUND failed!" << endl;
//
//    return 0;

    for (int i=69; i>0; i--){
        t.insert(i,0);
    }
    t.printTree();

    cout << "height" << endl;

}