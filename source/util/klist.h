#ifndef __KLIST_H__
#define __KLIST_H__

// specialized list so that no memory is allocated when an object is added or iterated
// 
// general the node will live as a member variable on the object it will contain
// because of this, it is bad to use BoxedPtr with this class because the node would then hold an extra reference
template <typename T>
class KList;

template <typename T>  
class KListNode : public BoxedPtrBase {
public:
    KListNode(T data) : data(data), prev(NULL), next(NULL), list(NULL) {}        
    T data;

    void remove() {        
        if (!this->list) {
            return;
        }
        if (this->prev) {
            this->prev->next = this->next;
        } else {
            this->list->first = this->next;
        }
        if (this->next) {
            this->next->prev = this->prev;
        } else {
            this->list->last = this->prev;
        }
        this->list->count--;
        this->list = NULL;
        this->next = NULL;
        this->prev = NULL;
    }
    
    bool isInList() {return list!=NULL;}

    KListNode<T>* getNext() {return this->next;}
private:
    friend KList<T>;
    KListNode<T>* prev;
    KListNode<T>* next;
    KList<T>* list;
}; 

template <typename T>  
class KList {
public:
    KList() : first(NULL), last(NULL), count(0) {}         
    ~KList() {
        KListNode<T>* n = first;
        while (n) {
            KListNode<T>* next = n->next;
            n->next = NULL;
            n->prev = NULL;
            n->list = NULL;
            n = next;
        }
    }
    bool isEmpty() {return first==NULL;}
    void addToBack(KListNode<T>* node) {
        if (node->list) {
            if (node->list == this) {
                kwarn("Node already in list");
            } else {
                kpanic("Node already in list");
            }
        }
        if (!this->last) {
            first = node;
            last = node;
        } else {
            node->prev = last;
            last->next = node;
            last = node;
        }
        node->list = this;
        count++;
    } 
    void addToFront(KListNode<T>* node) {
        if (node->list) {
            kpanic("Node already in list");
        }
        if (!this->first) {
            first = node;
            last = node;
        } else {
            node->next = first;
            first->prev = node;
            first = node;
        }
        node->list = this;
        count++;
    }
    void for_each(std::function<void(KListNode<T>*)> f) {
        KListNode<T>* node = first;
        while (node) {
            KListNode<T>* next = node->next;
            f(node);
            node = next;
        }
    }

    KListNode<T>* front() {return first;}
    KListNode<T>* back() {return last;}
    U32 size() {return this->count;}

private:
    friend KListNode<T>;
    KListNode<T>* first;
    KListNode<T>* last;
    U32 count;    
};

#endif
