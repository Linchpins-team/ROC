package rbtree

type Tree struct {
	Root *Node
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
	Root **Node
	ValueObject
}

func (tree *Tree) Insert(value ValueObject) {
	if tree.Root == nil {
		tree.Root = &Node{
			Color:       BLACK,
			ValueObject: value,
			Root: &tree.Root,
		}
	} else {
		tree.Root.Insert(value)
	}
}

func (tree *Tree) Search(key uint64) ValueObject {
	return tree.Root.Search(key)
}

func (tree *Tree) String() string {
	return tree.Root.String()
}

type ValueObject interface {
	Value() uint64
}

func New() *Tree {
	return &Tree{
		Root: nil,
	}
}

func (n *Node) Delete() {
	if n == nil {
		return
	}
	n.Right.Delete()
	n.Right = nil
	n.Left.Delete()
	n.Left = nil
	n.ValueObject = nil
}

func (n *Node) getRightMost() **Node {
	if n.Right == nil {
		return &n.Right
	}
	return n.Right.getRightMost()
}
