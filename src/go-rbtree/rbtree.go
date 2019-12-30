package rbtree

type RBTree struct {
	*Tree
}

type Tree struct {
	Left   *Tree
	Right  *Tree
	ValueObject
}

func (tree *RBTree) Insert(value ValueObject) {
	if tree.Tree == nil {
		tree.Tree = &Tree{
			ValueObject: value,
		}
	} else {
		tree.Tree.Insert(value)
	}
}

func (tree *RBTree) Delete(key uint64) {
	tree.Tree.Delete(key, &tree.Tree)
}

type ValueObject interface {
	Value() uint64
}

func New() *RBTree {
	return &RBTree{
		Tree: nil,
	}
}

func (tree *Tree) Delete(key uint64, refer **Tree) {
	if tree == nil {
		return
	}
	if key > tree.Value() {
		tree.Right.Delete(key, &tree.Right)
	} else if key < tree.Value() {
		tree.Left.Delete(key, &tree.Left)
	} else {
		tree.delete(refer)
	}
}

// delete this tree
func (tree *Tree) delete(refer **Tree) {
	if tree.Right == nil && tree.Left == nil {
		// Set parent's left or right node to nil
		*refer = nil
	} else if tree.Right != nil && tree.Left != nil {
		// Set left node to this position
		*refer = tree.Left
		rightMost := tree.Left.getRightMost()
		*rightMost = tree.Right
	} else if tree.Right != nil && tree.Left == nil {
		*refer = tree.Right
	} else if tree.Right == nil && tree.Left != nil {
		*refer = tree.Left
	}
	// C need to release memory
}

func (tree *Tree) getRightMost() **Tree {
	if tree.Right == nil {
		return &tree.Right
	}
	return tree.Right.getRightMost()
}

