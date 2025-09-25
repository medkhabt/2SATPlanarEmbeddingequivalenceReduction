#pragma once

#include <ogdf/basic/Graph_d.h>
#include <vector>

namespace ogdf {

    class NodePartition : protected GraphObserver {
        public:
            using element_type = node;
            using cell_elements_list = std::vector<element_type>;
            using partition_cells_list = std::vector<cell_elements_list>;

            //! Creates an empty partition associated with registry \p R.
            explicit NodePartition(const Graph& R) { init(R); }

            //! Creates an empty partition associated with no registry.
            explicit NodePartition() { }

            //! Reinitializes the partition. Associates the partition with no registry.
            void init() { // virtual until observers are fixed
                m_cell.init();
                m_pos.init();
                m_cells.clear();
                reregister(nullptr);
            }

            //! Reinitializes the partition. Associates the partition with registry \p R.
            void init(const Graph& R) { // virtual until observers are fixed
                m_cell.init(R, -1); // std::numeric_limits<size_t>::max()
                m_pos.init(R, -1);
                m_cells.resize(1);
                auto& cell = m_cells.back();
                cell.clear();
                cell.reserve(R.numberOfNodes());
                for (const auto& e : R.nodes) {
                    m_cell[e] = 0;
                    m_pos[e] = cell.size();
                    cell.push_back(e);
                }
                reregister(&R);
            }

            NodePartition& operator=(const partition_cells_list& assign) {
                while (assign.size() > m_cells.size()) {
                    newCell();
                }
                for (int i = 0; i < assign.size(); ++i) {
                    const cell_elements_list& C = assign[i];
                    m_cells.reserve(C.size());
                    for (int j = 0; j < C.size(); ++j) {
                        moveToCell(C[j], i);
                    }
                }
                while (assign.size() < m_cells.size()) {
                    delCell(m_cells.size() - 1);
                }
                consistencyCheck();
                return *this;
            }

            NodePartition& operator=(const NodeArray<int>& assign) {
                for (const auto& E : graphOf()->nodes) {
                    int cell = assign[E];
                    while (cell > m_cells.size()) {
                        newCell();
                    }
                    moveToCell(E, cell);
                }
                while (m_cells.back().empty()) {
                    delCell(m_cells.size() - 1);
                }
                consistencyCheck();
                return *this;
            }

            //! Moves all elements to a single cell, making this partition trivial
            void clear() {
                if (graphOf()) {
                    init(*graphOf());
                } else {
                    init();
                }
            }

            void newCell() {
                m_cells.emplace_back();
            }

            void delCell(int idx) {
                OGDF_ASSERT(cell(idx).empty());
                for (int i = idx + 1; i < m_cells.size(); ++i) {
                    for (const auto& e : m_cells[i - 1]) {
                        OGDF_ASSERT(m_cell[e] == i);
                        m_cell[e] = i - 1;
                    }
                }
                m_cells.erase(m_cells.begin() + idx);
            }

            std::pair<int, int> moveToCell(element_type key, int to_cell) {
                const auto& ret = elementRemoved(key);
                elementAdded(key, to_cell);
                return ret;
            }

            int moveToPos(element_type key, int to_pos) {
                int old_pos = m_pos[key];
                cell_elements_list& C = cell(m_cell[key]);
                OGDF_ASSERT(0 <= to_pos);
                OGDF_ASSERT(to_pos < C.size());

                // std::rotate(first, middle, last)
                // elements in [first, middle) are placed after the elements in [middle, last)
                if (to_pos > old_pos) {
                    std::rotate(C.begin() + old_pos, C.begin() + old_pos + 1, C.begin() + to_pos + 1);
                    for (int i = old_pos; i < to_pos; ++i) {
                        // std::cerr << i << " " << m_pos[C[i]] << std::endl;
                        OGDF_ASSERT(m_pos[C[i]] == i + 1);
                        m_pos[C[i]] = i;
                    }
                } else if (to_pos < old_pos) {
                    std::rotate(C.begin() + to_pos, C.begin() + old_pos, C.begin() + old_pos + 1);
                    for (int i = to_pos + 1; i <= old_pos; ++i) {
                        // std::cerr << i << " " << m_pos[C[i]] << std::endl;
                        OGDF_ASSERT(m_pos[C[i]] == i - 1);
                        m_pos[C[i]] = i;
                    }
                }
                OGDF_ASSERT(m_pos[C[to_pos]] == old_pos);
                m_pos[C[to_pos]] = to_pos;

                return old_pos;
            }

        protected:
            void elementAdded(element_type key, int to_cell = 0) {
                cell_elements_list& CC = m_cells.at(to_cell);
                m_cell[key] = to_cell;
                m_pos[key] = CC.size();
                CC.push_back(key);
            }

            std::pair<int, int> elementRemoved(element_type key) {
                int old_cell = m_cell[key];
                int old_pos = m_pos[key];
                cell_elements_list& C = cell(old_cell);
                C.erase(C.begin() + old_pos);
                for (int i = old_pos + 1; i <= C.size(); ++i) {
                    OGDF_ASSERT(m_pos[C[i - 1]] == i);
                    m_pos[C[i - 1]] = i - 1;
                }
                return {old_cell, old_pos};
            }

            void elementsCleared() {
                m_cells.resize(1);
                m_cells.front().clear();
            }

            cell_elements_list& cell(int idx) {
                return m_cells.at(idx);
            }

        public:
            int cellOf(element_type key) const { return m_cell[key]; }

            int positionOf(element_type key) const { return m_pos[key]; }

            const cell_elements_list& cell(int idx) const {
                return m_cells.at(idx);
            }

            //! Returns a reference to the list of cells that make up this partition
            const partition_cells_list& cells() const { return m_cells; }

            //! Returns a RegisteredArray that assigns the cell number to each element of the Registry
            const NodeArray<int>& cellAssignment() const { return m_cell; }

            //! Returns a RegisteredArray that assigns the position within its cell to each element of the Registry
            const NodeArray<int>& positionAssignment() const { return m_pos; }

            //! Returns the number of cells that make up this partition
            int size() const { return m_cells.size(); }

            //! Returns true if there are less than 2 cells
            bool isTrivial() const { return m_cells.size() < 2; }

            typename partition_cells_list::const_iterator begin() const { return m_cells.begin(); }

            typename partition_cells_list::const_iterator end() const { return m_cells.end(); }

            //! Returns the associated registry.
            const Graph* graphOf() const { return m_cell.graphOf(); }

            void consistencyCheck() {
                if (!graphOf()) {
                    OGDF_ASSERT(!m_cell.graphOf());
                    OGDF_ASSERT(!m_pos.graphOf());
                    OGDF_ASSERT(m_cells.empty());
                    return;
                }
                OGDF_ASSERT(m_cell.graphOf() == m_pos.graphOf());
                OGDF_ASSERT(!m_cells.empty());
                NodeArray<bool> seen(*graphOf(), false);
                int registry_size = 0;
                for (const auto e : graphOf()->nodes) {
                    OGDF_ASSERT(!seen[e]);
                    seen[e] = true;
                    registry_size++;
                }

                int partition_size = 0;
                for (int i = 0; i < m_cells.size(); ++i) {
                    const auto& C = m_cells[i];
                    for (int j = 0; j < C.size(); ++j) {
                        OGDF_ASSERT(m_cell[C[j]] == i);
                        OGDF_ASSERT(m_pos[C[j]] == j);
                        OGDF_ASSERT(seen[C[j]]);
                        seen[C[j]] = false;
                        partition_size++;
                    }
                }
                OGDF_ASSERT(registry_size == partition_size);
            }

        private:
            NodeArray<int> m_cell;
            NodeArray<int> m_pos;
            partition_cells_list m_cells;
        protected:
            void nodeDeleted(node v) override { elementRemoved(v); }

            void nodeAdded(node v) override { elementAdded(v); }

            void edgeDeleted(edge e) override { }

            void edgeAdded(edge e) override { }

            void cleared() override { elementsCleared(); }
    };

}

