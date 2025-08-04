#pragma once 
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphList.h>

#include <utility>
namespace ogdf{

    class PairNodeElement;
    using pairNode = ogdf::PairNodeElement*; 
    class GraphWithPairNode; 
    inline int szudzik_pairing(int x, int y){
        if(x < y){
            OGDF_ASSERT(y < INT_MAX / y);  
            OGDF_ASSERT(y*y < INT_MAX - x);
            return y*y + x; 
        } else {
            OGDF_ASSERT(x < INT_MAX / x);  
            OGDF_ASSERT(x*x < INT_MAX - y);
            return x*x + x + y;
        }
    }
    namespace internal {
        class OGDF_EXPORT GraphElementWithPairNode{ 
            friend class ogdf::Graph;
            friend class ogdf::GraphWithPairNode; 
            friend class GraphListBaseWithPairNode;

            protected:
            GraphElementWithPairNode* m_next = nullptr; //!< The successor in the list.
            GraphElementWithPairNode* m_prev = nullptr; //!< The predecessor in the list.

            OGDF_NEW_DELETE
        };

        class OGDF_EXPORT GraphListBaseWithPairNode {
            protected:
                int m_size; //!< The size of the list.
                GraphElementWithPairNode* m_head; //!< Pointer to the first element in the list.
                GraphElementWithPairNode* m_tail; //!< Pointer to the last element in the list.

            public:
                //! Constructs an empty list.
                GraphListBaseWithPairNode() {
                    m_head = m_tail = nullptr;
                    m_size = 0;
                }

                //! Destruction
                ~GraphListBaseWithPairNode() { }

                //! Returns the size of the list.
                int size() const { return m_size; }

                //! Returns true iff the list is empty.
                bool empty() const { return m_size == 0; }

                //! Adds element \p pX at the end of the list.
                void pushBack(GraphElementWithPairNode* pX) {
                    pX->m_next = nullptr;
                    pX->m_prev = m_tail;
                    if (m_head) {
                        m_tail = m_tail->m_next = pX;
                    } else {
                        m_tail = m_head = pX;
                    }
                    ++m_size;
                }

                //! Inserts element \p pX after element \p pY.
                void insertAfter(GraphElementWithPairNode* pX, GraphElementWithPairNode* pY) {
                    pX->m_prev = pY;
                    GraphElementWithPairNode* pYnext = pX->m_next = pY->m_next;
                    pY->m_next = pX;
                    if (pYnext) {
                        pYnext->m_prev = pX;
                    } else {
                        m_tail = pX;
                    }
                    ++m_size;
                }

                //! Inserts element \p pX before element \p pY.
                void insertBefore(GraphElementWithPairNode* pX, GraphElementWithPairNode* pY) {
                    pX->m_next = pY;
                    GraphElementWithPairNode* pYprev = pX->m_prev = pY->m_prev;
                    pY->m_prev = pX;
                    if (pYprev) {
                        pYprev->m_next = pX;
                    } else {
                        m_head = pX;
                    }
                    ++m_size;
                }

                //! Removes element \p pX from the list.
                void del(GraphElementWithPairNode* pX) {
                    GraphElementWithPairNode *pxPrev = pX->m_prev, *pxNext = pX->m_next;

                    if (pxPrev) {
                        pxPrev->m_next = pxNext;
                    } else {
                        m_head = pxNext;
                    }
                    if (pxNext) {
                        pxNext->m_prev = pxPrev;
                    } else {
                        m_tail = pxPrev;
                    }
                    m_size--;
                }

                //! Sorts the list according to \p newOrder.
                template<class LIST>
                    void sort(const LIST& newOrder) {
                        using std::begin;
                        using std::end;
                        sort(begin(newOrder), end(newOrder));
                    }

                //! Sorts the list according to the range defined by two iterators.
                template<class IT>
                    void sort(IT begin, IT end) {
                        if (begin == end) {
                            return;
                        }
                        m_head = *begin;
                        GraphElementWithPairNode* pPred = nullptr;
                        for (auto it = begin; it != end; ++it) {
                            GraphElementWithPairNode* p = *it;
                            if ((p->m_prev = pPred) != nullptr) {
                                pPred->m_next = p;
                            }
                            pPred = p;
                        }
                        (m_tail = pPred)->m_next = nullptr;
                    }

                //! Reverses the order of the list elements.
                void reverse() {
                    GraphElementWithPairNode* pX = m_head;
                    m_head = m_tail;
                    m_tail = pX;
                    while (pX) {
                        GraphElementWithPairNode* pY = pX->m_next;
                        pX->m_next = pX->m_prev;
                        pX = pX->m_prev = pY;
                    }
                }

                //! Exchanges the positions of \p pX and \p pY in the list.
                void swap(GraphElementWithPairNode* pX, GraphElementWithPairNode* pY) {
                    if (pX->m_next == pY) {
                        pX->m_next = pY->m_next;
                        pY->m_prev = pX->m_prev;
                        pY->m_next = pX;
                        pX->m_prev = pY;

                    } else if (pY->m_next == pX) {
                        pY->m_next = pX->m_next;
                        pX->m_prev = pY->m_prev;
                        pX->m_next = pY;
                        pY->m_prev = pX;

                    } else {
                        std::swap(pX->m_next, pY->m_next);
                        std::swap(pX->m_prev, pY->m_prev);
                    }

                    if (pX->m_prev) {
                        pX->m_prev->m_next = pX;
                    } else {
                        m_head = pX;
                    }
                    if (pX->m_next) {
                        pX->m_next->m_prev = pX;
                    } else {
                        m_tail = pX;
                    }

                    if (pY->m_prev) {
                        pY->m_prev->m_next = pY;
                    } else {
                        m_head = pY;
                    }
                    if (pY->m_next) {
                        pY->m_next->m_prev = pY;
                    } else {
                        m_tail = pY;
                    }

#ifdef OGDF_DEBUG
                    consistencyCheck();
#endif
                }

                //! Permutes all list elements.
                template<class RNG>
                    void permute(RNG& rng) {
                        Array<GraphElementWithPairNode*> A(m_size + 2);
                        A[0] = A[m_size + 1] = nullptr;

                        int i = 1;
                        GraphElementWithPairNode* pX;
                        for (pX = m_head; pX; pX = pX->m_next) {
                            A[i++] = pX;
                        }

                        A.permute(1, m_size, rng);

                        for (i = 1; i <= m_size; i++) {
                            pX = A[i];
                            pX->m_next = A[i + 1];
                            pX->m_prev = A[i - 1];
                        }

                        m_head = A[1];
                        m_tail = A[m_size];

#ifdef OGDF_DEBUG
                        consistencyCheck();
#endif
                    }

                //! Permutes all list elements.
                void permute() {
                    std::minstd_rand rng(randomSeed());
                    permute(rng);
                }

#ifdef OGDF_DEBUG
                //! Asserts consistency of this list.
                void consistencyCheck() const {
                    OGDF_ASSERT((m_head == nullptr) == (m_tail == nullptr));

                    if (m_head != nullptr) {
                        OGDF_ASSERT(m_head->m_prev == nullptr);
                        OGDF_ASSERT(m_tail->m_next == nullptr);

                        for (GraphElementWithPairNode* pX = m_head; pX; pX = pX->m_next) {
                            if (pX->m_prev) {
                                OGDF_ASSERT(pX->m_prev->m_next == pX);
                            } else {
                                OGDF_ASSERT(pX == m_head);
                            }

                            if (pX->m_next) {
                                OGDF_ASSERT(pX->m_next->m_prev == pX);
                            } else {
                                OGDF_ASSERT(pX == m_tail);
                            }
                        }
                    }
                }
#endif

                OGDF_NEW_DELETE
        };

        template<class T>
            class GraphListWithPairNode : protected GraphListBaseWithPairNode { 

                public:
                    //! The value type (a pointer to a specific graph object)
                    using value_type = T*;
                    //! Provides a bidirectional iterator to an object in the container.
                    using iterator = GraphIterator<T*>; // E: No template named 'GraphIterator'
                                                        //! Provides a bidirectional reverse iterator to an object in the container.
                    using reverse_iterator = GraphReverseIterator<T*>; // E: No template named 'GraphReverseIterator'

                    //! Constructs an empty list.
                    GraphListWithPairNode() { }

                    //! Destruction: deletes all elements
                    ~GraphListWithPairNode() {
                        if (m_head) {
                            OGDF_ALLOCATOR::deallocateList(sizeof(T), m_head, m_tail);
                        }
                    }

                    using GraphListBaseWithPairNode::empty; // E: Using declaration requires a qualified name
                    using GraphListBaseWithPairNode::size; // E: Using declaration requires a qualified name

                    //! Returns the first element in the list.
                    T* head() const { return static_cast<T*>(m_head); }

                    //! Returns the last element in the list.
                    T* tail() const { return static_cast<T*>(m_tail); }

                    //! Adds element \p pX at the end of the list.
                    void pushBack(T* pX) { GraphListBaseWithPairNode::pushBack(pX); }

                    //! Inserts element \p pX after element \p pY.
                    void insertAfter(T* pX, T* pY) { GraphListBaseWithPairNode::insertAfter(pX, pY); }

                    //! Inserts element \p pX before element \p pY.
                    void insertBefore(T* pX, T* pY) { GraphListBaseWithPairNode::insertBefore(pX, pY); }

                    //! Moves element \p pX to list \p L and inserts it before or after \p pY.
                    void move(T* pX, GraphList<T>& L, T* pY, Direction dir) { // E: Unknown type name 'Direction'
                        GraphListBaseWithPairNode::del(pX);
                        if (dir == Direction::after) {
                            L.insertAfter(pX, pY);
                        } else {
                            L.insertBefore(pX, pY);
                        }
                    }

                    //! Moves element \p pX to list \p L and inserts it at the end.
                    void move(T* pX, GraphList<T>& L) {
                        GraphListBaseWithPairNode::del(pX);
                        L.pushBack(pX);
                    }

                    //! Moves element \p pX from its current position to a position after \p pY.
                    void moveAfter(T* pX, T* pY) {
                        GraphListBaseWithPairNode::del(pX);
                        insertAfter(pX, pY);
                    }

                    //! Moves element \p pX from its current position to a position before \p pY.
                    void moveBefore(T* pX, T* pY) {
                        GraphListBaseWithPairNode::del(pX);
                        insertBefore(pX, pY);
                    }

                    //! Removes element \p pX from the list and deletes it.
                    void del(T* pX) {
                        GraphListBaseWithPairNode::del(pX);
                        delete pX;
                    }

                    //! Only removes element \p pX from the list; does not delete it.
                    void delPure(T* pX) { GraphListBaseWithPairNode::del(pX); }

                    //! Removes all elements from the list and deletes them.
                    void clear() {
                        if (m_head) {
                            OGDF_ALLOCATOR::deallocateList(sizeof(T), m_head, m_tail);
                            m_head = m_tail = nullptr;
                            m_size = 0;
                        }
                    }
                    //! Returns an iterator to the first element in the container.
                    iterator begin() const { return GraphListWithPairNode<T>::head(); } // E: Unknown type name 'iterator'

                    //! Returns an iterator to the one-past-last element in the container.
                    iterator end() const { return iterator(); }

                    //! Returns a reverse iterator to the last element in the container.
                    reverse_iterator rbegin() const { return reverse_iterator(GraphListWithPairNode<T>::tail()); }

                    //! Returns a reverse iterator to the one-before-first element in the container.
                    reverse_iterator rend() const { return reverse_iterator(); }

                    using GraphListBaseWithPairNode::permute;
                    using GraphListBaseWithPairNode::reverse;
                    using GraphListBaseWithPairNode::sort;

                    //! Exchanges the positions of \p pX and \p pY in the list.
                    void swap(T* pX, T* pY) { GraphListBaseWithPairNode::swap(pX, pY); }

#ifdef OGDF_DEBUG
                    using GraphListBaseWithPairNode::consistencyCheck;
#endif
            };
        template<class GraphObject>
            class GraphObjectContainerWithGraphWithPairNode : private GraphListWithPairNode<GraphObject> {
                friend class ogdf::Graph;
                friend class ogdf::ClusterGraph;
                friend class ogdf::ConstCombinatorialEmbedding;
                friend class ogdf::CombinatorialEmbedding;
                friend class ogdf::GraphWithPairNode;

                public:
                using typename GraphListWithPairNode<GraphObject>::value_type;
                using typename GraphListWithPairNode<GraphObject>::iterator;
                using typename GraphListWithPairNode<GraphObject>::reverse_iterator;

                using GraphListWithPairNode<GraphObject>::begin;
                using GraphListWithPairNode<GraphObject>::rbegin;
                using GraphListWithPairNode<GraphObject>::end;
                using GraphListWithPairNode<GraphObject>::rend;

                using GraphListWithPairNode<GraphObject>::size;
                using GraphListWithPairNode<GraphObject>::empty;
                using GraphListWithPairNode<GraphObject>::head;
                using GraphListWithPairNode<GraphObject>::tail;
            };
    }

    class OGDF_EXPORT PairNodeElement: public internal::GraphElementWithPairNode {
        friend class GraphWithPairNode;
        friend class GraphListBaseWithPairNode;
        node m_first; 
        node m_second;
        int m_id;
#ifdef OGDF_DEBUG
        const Graph* m_pGraph;
#endif

#ifdef OGDF_DEBUG

        PairNodeElement(const Graph* pGraph, node firstNode, node secondNode)
            : m_pGraph(pGraph), m_first(firstNode), m_second(secondNode){
                m_id = szudzik_pairing(firstNode->index(), secondNode->index());
            }
#else
        PairNodeElement(node firstNode, node secondNode)
            :m_id(id), m_first(firstNode), m_second(secondNode){
                m_id = szudzik_pairing(firstNode->index(), secondNode->index());
            }
#endif

        public:
        int index(){ return m_id; }
        node firstNode() {return m_first;}
        node secondNode() {return m_second;}
        pairNode succ() const { return static_cast<pairNode>(m_next); }
        pairNode pred() const { return static_cast<pairNode>(m_prev); }

#ifdef OGDF_DEBUG
        const Graph* graphOf() const { return m_pGraph; }
#endif
        // TODO I don't know yet the impact of the ordering for this class.
        static int compare(const PairNodeElement& x, const PairNodeElement& y) { return x.m_id - y.m_id; }
        OGDF_AUGMENT_STATICCOMPARER(PairNodeElement)

            OGDF_NEW_DELETE
    };
}
