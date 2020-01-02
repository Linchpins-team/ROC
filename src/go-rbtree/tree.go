package rbtree

// Tree contains the NIL node of a tree, the root node is the left child of NIL
type Tree struct {
	workNode
}

const (
	RED   bool = false
	BLACK      = true
)

type Node struct {
	Parent *Node
	Left   *Node
	Right  *Node
	Color  bool
	ValueObject
}

func (tree Tree) work() workNode {
	return workNode{
		NIL:  tree.Node,
		Node: tree.Node,
	}
}

func (tree Tree) String() string {
	return tree.left().String()
}

func (tree Tree) Search(key uint64) ValueObject {
	return tree.left().Search(key)
}

func (tree Tree) Delete() {
	tree.left().Delete()
}

func (tree Tree) Check() {
	tree.left().Count()
}

type ValueObject interface {
	Value() uint64
}

func New() Tree {
	NIL := new(Node)
	NIL.Color = BLACK
	tree := Tree{
		workNode{
			Node: NIL,
			NIL:  NIL,
		},
	}
	return tree
}

func (n workNode) Delete() {
	if n.Node == nil {
		return
	}
	n.right().Delete()
	n.Right = nil
	n.left().Delete()
	n.Left = nil
	n.ValueObject = nil
}
