#pragma once 
#include <ogdf/basic/Graph.h>
#include "type.hpp"
#include "GraphObjectContainerWithGraphWithPairNode.hpp"
namespace ogdf {
    class GraphWithPairNode; 
    namespace internal {
        using GraphPairNodeRegistry  = GraphRegistry<PairNodeElement>; 
        template<class CONTAINER>
            inline void getAllPairContainer(const ogdf::GraphWithPairNode& G, CONTAINER& pairNodes);
    }
    // TODO i have to implement the graph registered Array
    template<class Registry, class Value, bool WithDefault = true>
    class GraphWithPairNodeRegisteredArray : public RegisteredArray<Registry, Value, WithDefault, GraphWithPairNode>{
    
    };

    class OGDF_EXPORT GraphWithPairNode: public Graph {
        internal::GraphPairNodeRegistry m_regPairNodeArrays;  
        public:
        using pair_node_iterator = internal::GraphIterator<pairNode>;
        internal::GraphObjectContainerWithGraphWithPairNode<PairNodeElement> pairsOfNodes; 

        GraphWithPairNode();


        OGDF_COPY_CONSTR(GraphWithPairNode);


        OGDF_COPY_OP(GraphWithPairNode);

        OGDF_NO_MOVE(GraphWithPairNode);

        virtual ~GraphWithPairNode();

        OGDF_MALLOC_NEW_DELETE

        int numberOfPairNodes() const { return pairsOfNodes.size(); }
        pairNode firstPairNode() const { return pairsOfNodes.head(); }
        pairNode lastPairNode() const { return pairsOfNodes.tail(); }
        pairNode choosePairNode(
                std::function<bool(pairNode)> includePairNode = [](pairNode) { return true; },
                bool isFastTest = true) const;

        // TODO do i really need a container ? for easier loop but i don't think i need it now.
        template<class CONTAINER>
            void allPairNodes(CONTAINER& pairNodeContainer) const {
                internal::getAllPairContainer<CONTAINER>(*this, pairNodeContainer);
            }
        pairNode newPairNode(node u, node v){
            pairNode pn = pureNewPairNode(u,v);
            m_regPairNodeArrays.keyAdded(pn);

            /*
             * TODO I don't think I need an observer for the pairNodes. 
            for (GraphObserver* obs : getObservers()) {
                obs->pairNodeAdded(v);
            }
            */
            return pn;
        }
        node newNode(int index = -1) {
            node v = Graph::newNode(index);
            for(node n: this->nodes){
                pairNode vn = newPairNode(v,n);  
                if(v->index() != n->index())
                    pairNode nv = newPairNode(n,v);  
            }
            for (GraphObserver* obs : getObservers()) {
                obs->nodeAdded(v);
            }
            return v;
        }
        private:
        inline pairNode pureNewPairNode(node u, node v) {
            OGDF_ASSERT(u != nullptr);
            OGDF_ASSERT(v != nullptr);
            OGDF_ASSERT(u->graphOf() == this);
            OGDF_ASSERT(v->graphOf() == this);

 #ifdef OGDF_DEBUG
         pairNode uv = new PairNodeElement(this, u, v);
 #else
         pairNode uv = new PairNodeElement(u, v);
 #endif
         pairsOfNodes.pushBack(uv);
         return uv;
        }
    };
    namespace internal{
        template<typename CONTAINER>
            inline void getAllPairContainer(const ogdf::GraphWithPairNode& G, CONTAINER& pairNodes) {
                pairNodes.clear();
                for (pairNode p : G.pairsOfNodes) {
                    pairNodes.pushBack(p);
                }
            }
    }
}
