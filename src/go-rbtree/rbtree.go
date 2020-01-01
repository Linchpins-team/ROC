package rbtree

type Tree struct {
	NIL *Node
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

func (tree *Tree) Insert(value ValueObject) {
	tree.NIL.Insert(value, tree)
}

func (tree *Tree) Search(key uint64) ValueObject {
	return tree.NIL.Left.Search(key, tree)
}

func (tree *Tree) String() string {
	return tree.NIL.Left.Work(tree.NIL).String()
}

func (tree *Tree) Delete() {
	tree.NIL.Left.Delete(tree)
}

func (tree *Tree) Check() {
	tree.NIL.Left.Work(tree.NIL).Count()
}

type ValueObject interface {
	Value() uint64
}

func New() *Tree {
	NIL := new(Node)
	NIL.Left = NIL
	NIL.Right = NIL
	NIL.Color = BLACK
	tree := &Tree{
		NIL: NIL,
	}
	return tree
}

func (n *Node) Delete(tree *Tree) {
	if n == tree.NIL {
		return
	}
	n.Right.Delete(tree)
	n.Right = nil
	n.Left.Delete(tree)
	n.Left = nil
	n.ValueObject = nil
}

func (n *Node) getRightMost() **Node {
	if n.Right == nil {
		return &n.Right
	}
	return n.Right.getRightMost()
}
