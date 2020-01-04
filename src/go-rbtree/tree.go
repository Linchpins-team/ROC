package rbtree

// Tree contains the NIL node of a tree, the root node is the left child of NIL
type Tree interface {
	Insert(value ValueObject)
	Search(key uint64) ValueObject
	Delete()
	String() string
	Count() int
}

type workNode struct {
	NIL *Node
	*Node
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

type ValueObject interface {
	Value() uint64
}

func New() Tree {
	NIL := new(Node)
	NIL.Color = BLACK
	return workNode{
		Node: NIL,
		NIL:  NIL,
	}
}
