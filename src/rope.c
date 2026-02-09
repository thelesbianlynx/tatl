#include "rope.h"

#include "array.h"
#include "intbuffer.h"
#include "charbuffer.h"

typedef struct content Content;

enum {
    NODE_INTERNAL,
    NODE_CONTENT,
};

struct content {
    uint32_t rc;
    uint32_t len;
    uint32_t lines;
    uint32_t chars[NODE_CONTENT_SIZE];
};

struct rope_node {
    uint32_t type;
    uint32_t rc;
    union {
        struct {
            Node* child[4];
            uint32_t count;
        };
        struct {
            Content* content;
        };
    };

    uint32_t len;
    uint32_t lines;
    uint32_t level;
    uint32_t rem;
};


//
// Content Object.
//


static
Content* content_create () {
    Content* content = malloc(sizeof(Content));
    content->rc = 1;
    content->len = 0;
    content->lines = 0;
    return content;
}

// static
// void content_ref (Content* content) {
//     content->rc++;
// }

static
void content_unref (Content* content) {
    content->rc--;

    if (content->rc == 0) {
        free(content);
    }
}

static
void content_put (Content* content, int32_t* p, uint32_t len) {
    if (content->len + len > NODE_CONTENT_SIZE) return;

    for (int i = 0; i < len; i++) {
        content->chars[content->len + i] = p[i];
        if (p[i] == '\n') content->lines++;
    }

    content->len += len;
}

static
uint32_t content_rem (Content* content) {
    uint32_t rem = 0;
    for (int i = 0; i < content->len; i++) {
        char c = content->chars[i];
        if (c == '\n')
            rem = 0;
        else
            rem++;
    }
    return rem;
}


//
// Node Object.
//

static
Node* node_create_internal () {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_INTERNAL;
    node->rc = 1;
    for (int i = 0; i < 4; i++)
        node->child[i] = NULL;
    node->len = 0;
    node->lines = 0;
    node->level = 0;
    return node;
}

static
Node* node_create_content (Content* content) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_CONTENT;
    node->rc = 1;
    node->content = content;
    node->len = content->len;
    node->lines = content->lines;
    node->level = 0;
    node->rem = content_rem(content);
    return node;
}

static
void node_ref (Node* node) {
    node->rc++;
}

static
void node_unref (Node* node) {
    node->rc--;

    if (node->rc == 0) {
        if (node->type == NODE_CONTENT) {
            content_unref(node->content);
        } else if (node->type == NODE_INTERNAL) {
            for (int i = 0; i < 4; i++)
                if (node->child[i] != NULL)
                    node_unref(node->child[i]);
        }

        free(node);
    }
}

static
void node_sum (Node* node) {
    if (node->type != NODE_INTERNAL)
        return;

    uint32_t len = 0;
    uint32_t lines = 0;
    uint32_t level = 0;
    uint32_t count = 0;
    uint32_t rem = 0;

    for (int i = 0; i < 4; i++) {
        if (node->child[i] != NULL) {
            len += node->child[i]->len;
            lines += node->child[i]->lines;
            level = node->child[i]->level + 1;
            if (node->child[i]->lines > 0) rem = 0;
            rem += node->child[i]->rem;
            count++;
        } else break;
    }

    assert(level > 0 && count >= 2 && "Invalid Internal Node");

    node->len = len;
    node->lines = lines;
    node->level = level;
    node->count = count;
    node->rem = rem;
}

//
// Rope Object.
//

static
Rope* rope_new (Node* node) {
    Rope* r = malloc(sizeof(Rope));
    r->node = node;
    return r;
}

static
void split_buffer (IntBuffer* src, Array* A) {
    for (int i = 0; i < src->size; i += NODE_CONTENT_SIZE/2) {
        Content* cont = content_create();
        if (src->size - i < NODE_CONTENT_SIZE) {
            content_put(cont, src->data + i, src->size - i);
            Node* n = node_create_content(cont);
            array_add(A, n);
            return;
        } else {
            content_put(cont, src->data + i, NODE_CONTENT_SIZE/2);
            Node* n = node_create_content(cont);
            array_add(A, n);
        }
    }
}

Rope* rope_create (IntBuffer* src) {
    Array* A = array_create();
    Array* B = array_create();

    split_buffer(src, A);

    for (;;) {

        if (A->size == 1) {
            Rope* r = rope_new(A->data[0]);
            array_destroy(A);
            array_destroy(B);
            return r;
        }

        for (int i = 0; i < A->size; i += 2) {
            if (A->size - i == 1) {
                // Illegal state.
                assert(0 && "Invalid Rope: Only 1 intermediate node.");
            }
            if (A->size - i == 3) {
                // Special case: exactly 3 remaining nodes.

                Node* node = node_create_internal();
                node->child[0] = A->data[i];
                node->child[1] = A->data[i+1];
                node->child[2] = A->data[i+2];
                node_sum(node);
                array_add(B, node);
                break;
            }

            // Normal Case: exactly 2 or at least 4 remaining nodes.

            Node* node = node_create_internal();
            node->child[0] = A->data[i];
            node->child[1] = A->data[i+1];
            node_sum(node);
            array_add(B, node);
        }

        assert(B->size < A->size && "Rope Create: Infinite loop detected");

        // Clear and Swap arrays for next round.
        array_clear(A);
        Array* tmp = A;
        A = B;
        B = tmp;
    }
}

Rope* rope_copy (Rope* rope) {
    if (rope->node != NULL)
        node_ref(rope->node);

    return rope_new(rope->node);
}

void rope_destroy (Rope* rope) {
    if (rope->node != NULL)
        node_unref(rope->node);

    free(rope);
}


uint32_t rope_len (Rope* rope) {
    return rope->node == NULL ? 0 : rope->node->len;
}

uint32_t rope_lines (Rope* rope) {
    return rope->node == NULL ? 0 : rope->node->lines;
}


//
// Get Character.
//

uint32_t rope_get_char (Rope* rope){

    return 0;
}


//
// Split and Join.
//

static inline
uint32_t peek_level (Array* A) {
    assert(A->size > 0);
    return ((Node*)A->data[A->size - 1])->level;
}

static
void gather_nodes (Array** A, Array** B) {
    // Collect Contents of Arrays.
    //  Same process as in rope-create.

    for (int i = 0; i < (*A)->size; i += 2) {
        if ((*A)->size - i == 1) {
            // Illegal state.
            assert(0 && "Invalid Rope: Only 1 intermediate node.");
        }
        if ((*A)->size - i == 3) {
            // Special case: exactly 3 remaining nodes.
            Node* node = node_create_internal();
            node->child[0] = (*A)->data[i];
            node->child[1] = (*A)->data[i+1];
            node->child[2] = (*A)->data[i+2];
            node_sum(node);
            array_add(*B, node);
            break;
        }
        // Normal Case: exactly 2 or at least 4 remaining nodes.
        Node* node = node_create_internal();
        node->child[0] = (*A)->data[i];
        node->child[1] = (*A)->data[i+1];
        node_sum(node);
        array_add(*B, node);
    }
    // Clear and Swap arrays
    array_clear(*A);
    Array* tmp = *A;
    *A = *B;
    *B = tmp;
}

static
void gather_nodes_reverse (Array** A, Array** B) {
    // Collect Contents of Arrays (Reverse order).
    //  This version of the function handles the case
    //  where the nodes in the array in reverse order.

    for (int i = 0; i < (*A)->size; i += 2) {
        if ((*A)->size - i == 1) {
            // Illegal state.
            assert(0 && "Invalid Rope: Only 1 intermediate node.");
        }
        if ((*A)->size - i == 3) {
            // Special case: exactly 3 remaining nodes.
            Node* node = node_create_internal();
            node->child[0] = (*A)->data[i+2];
            node->child[1] = (*A)->data[i+1];
            node->child[2] = (*A)->data[i];
            node_sum(node);
            array_add(*B, node);
            break;
        }
        // Normal Case: exactly 2 or at least 4 remaining nodes.
        Node* node = node_create_internal();
        node->child[0] = (*A)->data[i+1];
        node->child[1] = (*A)->data[i];
        node_sum(node);
        array_add(*B, node);
    }
    // Clear and Swap arrays
    array_clear(*A);
    Array* tmp = *A;
    *A = *B;
    *B = tmp;
}

static
void node_prefix (Node* node, uint32_t i, uint32_t offset, Array** A, Array** B) {
    // Note: Reverse-Order iteration - offset is the *end* of the node
    if (offset > i) {
        // End of node is after the boundary.
        //  This means all previous nodes must have been after the boundary too.
        assert((*A)->size == 0 && "Non-empty array");

        if (offset - node->len >= i) {
            // Beginging of node is after the boundary.
            //  Node is completely outside the boundary.
            return;
        } else {
            // Beginging of node is before the boundary
            //  Partial coverage.
            if (node->type == NODE_CONTENT) {
                // Partial content node: create new content node that is a prefix of the old.
                int x = offset - i;
                Content* content = content_create();
                content_put(content, (int32_t*) (node->content->chars), node->content->len - x);

                Node* n = node_create_content(content);
                array_add(*A, n);
            } else if (node->type == NODE_INTERNAL) {
                // Partial internal node: recurse over children
                for (int x = node->count - 1; x >= 0; x--) {
                    node_prefix(node->child[x], i, offset, A, B);
                    offset -= node->child[x]->len;
                }

                // Array Handling:
                while ((*A)->size > 1) {
                    gather_nodes_reverse(A, B);
                }
            }
        }
    } else {
        // End of node is before the boundary.
        //  Node is completely inside the boundary.

        if ((*A)->size == 0) {
            // Special Case, i is on the boundary between nodes.
            node_ref(node);
            array_add(*A, node);
        } else {
            // Not the first node in the array -> Not the first node in the bounds.
            // Can only add self to list if it is the same level as the existing nodes.
            int level = peek_level(*A);
            if (node->level == level) {
                node_ref(node);
                array_add(*A, node);
            } else {
                assert(node->level > level && "Invalid level");
                // Greater level: must recurse until right level.
                for (int x = node->count - 1; x >= 0; x--) {
                    node_prefix(node->child[x], i, offset, A, B);
                    offset -= node->child[x]->len;
                }

                gather_nodes_reverse(A, B);
            }
        }
    }
}

static
void node_suffix (Node* node, uint32_t i, uint32_t offset, Array** A, Array** B) {
    if (offset < i) {
        // Beginging of node is before the boundary.
        //  This means all previous nodes must have been before the boundary too.
        assert((*A)->size == 0 && "Non-empty array");

        if (offset + node->len <= i) {
            // End of node is before the boundary.
            //  Node is completely outside the boundary.
            return;
        } else {
            // End of node is after the boundary
            //  Partial coverage.
            if (node->type == NODE_CONTENT) {
                // Partial content node: create new content node that is a suffix of the old.
                int x = i - offset;
                Content* content = content_create();
                content_put(content, (int32_t*) (node->content->chars + x), node->content->len - x);

                Node* n = node_create_content(content);
                array_add(*A, n);
            } else if (node->type == NODE_INTERNAL) {
                // Partial internal node: recurse over children
                for (int x = 0; x < node->count; x++) {
                    node_suffix(node->child[x], i, offset, A, B);
                    offset += node->child[x]->len;
                }

                // Array Handling:
                while ((*A)->size > 1) {
                    gather_nodes(A, B);
                }
            }
        }
    } else {
        // Beginging of node is after the boundary.
        //  Node is completely inside the boundary.

        if ((*A)->size == 0) {
            // Special Case, i is on the boundary between nodes.
            node_ref(node);
            array_add(*A, node);
        } else {
            // Not the first node in the array -> Not the first node in the bounds.
            // Can only add self to list if it is the same level as the existing nodes.
            int level = peek_level(*A);
            if (node->level == level) {
                node_ref(node);
                array_add(*A, node);
            } else {
                assert(node->level > level && "Invalid level");
                // Greater level: must recurse until right level.
                for (int x = 0; x < node->count; x++) {
                    node_suffix(node->child[x], i, offset, A, B);
                    offset += node->child[x]->len;
                }

                gather_nodes(A, B);
            }
        }
    }
}

Rope* rope_prefix (Rope* rope, uint32_t i) {
    if (rope->node == NULL) return rope_new(NULL);

    Array* A = array_create();
    Array* B = array_create();

    node_prefix(rope->node, i, rope->node->len, &A, &B);
    Rope* r = A->size > 0 ? rope_new(A->data[0]) : rope_new(NULL);

    array_destroy(A);
    array_destroy(B);
    return r;
}

Rope* rope_suffix (Rope* rope, uint32_t i) {
    if (rope->node == NULL) return rope_new(NULL);

    Array* A = array_create();
    Array* B = array_create();

    node_suffix(rope->node, i, 0, &A, &B);
    Rope* r = A->size > 0 ? rope_new(A->data[0]) : rope_new(NULL);

    array_destroy(A);
    array_destroy(B);
    return r;
}

Rope* rope_substr (Rope* rope, uint32_t i, uint32_t j) {
    Rope* a = rope_suffix(rope, i);
    Rope* b = rope_prefix(a, j-i);
    rope_destroy(a);
    return b;
}

static
Node* node_left_side (Node* node, Array* L) {
    while (node->type == NODE_INTERNAL) {
        array_add(L, node);
        node = node->child[0];
    }

    assert(node->type == NODE_CONTENT);
    return node;
}

static
Node* node_right_side (Node* node, Array* R) {
    while (node->type == NODE_INTERNAL) {
        array_add(R, node);
        assert(node->count >= 2);
        node = node->child[node->count-1];
    }

    assert(node->type == NODE_CONTENT);
    return node;
}

static
void merge2 (Node* a, Node* b, Array* A) {
    IntBuffer* src = intbuffer_create();
    for (int i = 0; i < a->len; i++)
        intbuffer_put_char(src, src->size, a->content->chars[i]);
    for (int i = 0; i < b->len; i++)
        intbuffer_put_char(src, src->size, b->content->chars[i]);
    split_buffer(src, A);
    intbuffer_destroy(src);
}

static
void merge3 (Node* a, Node* b, Node* c, Array* A) {
    IntBuffer* src = intbuffer_create();
    for (int i = 0; i < a->len; i++)
        intbuffer_put_char(src, src->size, a->content->chars[i]);
    for (int i = 0; i < b->len; i++)
        intbuffer_put_char(src, src->size, b->content->chars[i]);
    for (int i = 0; i < c->len; i++)
        intbuffer_put_char(src, src->size, c->content->chars[i]);
    split_buffer(src, A);
    intbuffer_destroy(src);
}

Rope* rope_append (Rope* a, Rope* b) {
    // Very Simple case.
    if (a->node == NULL && b->node == NULL) {
        return rope_new(NULL);
    } else if (a->node == NULL) {
        node_ref(b->node);
        return rope_new(b->node);
    } else if (b->node == NULL) {
        node_ref(a->node);
        return rope_new(a->node);
    }

    Array* L = array_create();
    Array* R = array_create();

    Node* l = node_right_side(a->node, L);
    Node* r = node_left_side(b->node, R);

    Array* A = array_create();
    Array* B = array_create();

    /* if (l->len >= NODE_CONTENT_SIZE/2 && r->len >= NODE_CONTENT_SIZE/2) {
        // Simple Join.
        // No need to merge any content nodes.


    } else */ if (l->len + r->len >= NODE_CONTENT_SIZE/2) {
        // Simple Merge.
        merge2(l,r, A);
    } else {
        // Complicated Merge.
        if (L->size == 0 || R->size == 0) {
            // If either array is length 0, then that rope was a single-content-node rope.
            // Simple merge in this case will still leave middle content nodes adequate length.
            // (And will leave at least one array at the right level for later gathering)
            merge2(l, r, A);
        } else {
            // Merge with Siblings (Right side).
            Node* n = array_pop(R);

            // (Count must be >= 2 for all internal nodes)
            merge3(l, r, n->child[1], A);

            // Add Siblings to staging array.
            for (int i = 2; i < n->count; i++){
                node_ref(n->child[i]);
                array_add(A, n->child[i]);
            }
        }

    }

    while (L->size > 0 || R->size > 0) {
        // Swap A to B; make A the staging array.
        Array* tmp = A;
        A = B;
        B = tmp;

        // Level of Nodes.
        int level = peek_level(B);

        // Add Left Siblings.
        //  (peek_level reads level from last node)
        if (L->size > 0 && peek_level(L) == level + 1) {
            Node* n = array_pop(L);
            for (int i = 0; i < n->count-1; i++){
                node_ref(n->child[i]);
                array_add(A, n->child[i]);
            }
        }

        // Add intermediate nodes from last gather.
        for (int i = 0; i < B->size; i++) {
            array_add(A, B->data[i]);
        }
        array_clear(B);

        // Add Right Siblings.
        if (R->size > 0 && peek_level(R) == level + 1) {
            Node* n = array_pop(R);
            for (int i = 1; i < n->count; i++){
                node_ref(n->child[i]);
                array_add(A, n->child[i]);
            }
        }

        // Gather.
        gather_nodes(&A, &B);
    }

    // Gather Until Root.
    while (A->size > 1) {
        gather_nodes(&A, &B);
    }

    // Return new rope.
    Rope* rope = rope_new(A->data[0]);

    array_destroy(A);
    array_destroy(B);
    array_destroy(L);
    array_destroy(R);
    return rope;
}


//
// Point-Index Conversion
//

Point node_index_to_point (Node* node, uint32_t index, uint32_t offset, uint32_t lines, uint32_t rem) {
    if (index >= offset + node->len) return (Point) {node->lines, node->rem};

    if (node->type == NODE_CONTENT) {
        int32_t col = rem;
        int32_t row = lines;
        for (int i = 0; i < node->len; i++) {
            if (offset + i == index)
                return (Point) {row, col};
            uint32_t c = node->content->chars[i];
            if (c == '\n') {
                row++;
                col = 0;
            } else {
                col++;
            }
        }
    } else {
        for (int i = 0; i < node->count; i++) {
            Node* n = node->child[i];
            if (offset <= index && index < offset + n->len)
                return node_index_to_point(n, index, offset, lines, rem);
            offset += n->len;
            lines += n->lines;
            if (n->lines > 0) rem = 0;
            rem += n->rem;
        }
    }

    assert(0 && "Index-to-Line Error");
    return (Point) {0,0};
}

Point rope_index_to_point (Rope* rope, int32_t index) {
    if (index < 0 || rope->node == NULL) return (Point) {0,0};

    return node_index_to_point(rope->node, index, 0, 0, 0);
}

// Returns the index of the first character of the specified line.
//  (i.e. the first character after the newline).
static
uint32_t node_line_to_index (Node* node, uint32_t offset, uint32_t lines, int32_t line) {
    if (line <= 0) return 0;
    if (node->lines + lines < line) return node->len;

    if (node->type == NODE_CONTENT) {
        for (int i = 0; i < node->len; i++) {
            uint32_t c = node->content->chars[i];
            if (c == '\n') {
                lines++;
                if (lines == line) return i + offset + 1;
            }
        }
    } else {
        for (int i = 0; i < node->count; i++) {
            Node* n = node->child[i];
            if (n->lines + lines >= line) {
                return node_line_to_index(n, offset, lines, line);
            }
            offset += n->len;
            lines += n->lines;
        }
    }

    assert(0 && "Line-to-Index Error");
    return 0;
}

uint32_t rope_point_to_index (Rope* rope, Point point) {
    if (rope->node == NULL) return 0;

    uint32_t l1 = node_line_to_index(rope->node, 0, 0, point.row);
    uint32_t l2 = node_line_to_index(rope->node, 0, 0, point.row + 1);

    return MIN(l1 + point.col, l2 - 1);
}

//
// For-Each.
//

static
bool node_foreach (Node* node, uint32_t i, uint32_t j, uint32_t offset, rope_foreach_fn fn, void* data) {
    if (node->type == NODE_CONTENT) {
        for (int x = i < offset ? 0 : i - offset; x < node->content->len && x + offset < j; x++) {
            if (!fn(x + offset, node->content->chars[x], data)) return false;
        }
    } else if (node->type == NODE_INTERNAL) {
        for (int x = 0; x < node->count; x++) {
            if (i < offset + node->child[x]->len) {
                if (!node_foreach(node->child[x], i, j, offset, fn, data)) {
                    return false;
                }
            }

            offset += node->child[x]->len;
            if (offset >= j) return false;
        }
    }
    return true;
}

static
bool node_foreach_reverse (Node* node, uint32_t i, uint32_t j, uint32_t offset, rope_foreach_fn fn, void* data) {
    if (node->type == NODE_CONTENT) {
        for (int x = j >= offset ? node->len - 1 : node->len - (offset - j) - 1; x >= 0 && offset - (node->len - x) >= i; x--) {
            if (!fn(offset - (node->len - x), node->content->chars[x], data)) return false;
        }
    } else if (node->type == NODE_INTERNAL) {
        for (int x = node->count - 1; x >= 0; x--) {
            if (j >= offset - node->child[x]->len) {
                if (!node_foreach_reverse(node->child[x], i, j, offset, fn, data)) {
                    return false;
                }
            }
            offset -= node->child[x]->len;
            if (offset < i) return false;
        }
    }
    return true;
}

void rope_foreach (Rope* rope, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach(rope->node, 0, rope->node->len, 0, fn, data);
}

void rope_foreach_prefix (Rope* rope, uint32_t i, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach(rope->node, 0, i, 0, fn, data);
}

void rope_foreach_suffix (Rope* rope, uint32_t i, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach(rope->node, i, rope->node->len, 0, fn, data);
}

void rope_foreach_substr (Rope* rope, uint32_t i, uint32_t j, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach(rope->node, i, j, 0, fn, data);
}


void rope_foreach_reverse (Rope* rope, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach_reverse(rope->node, 0, rope->node->len, rope->node->len, fn, data);
}

void rope_foreach_reverse_prefix (Rope* rope, uint32_t i, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach_reverse(rope->node, 0, i, rope->node->len, fn, data);
}

void rope_foreach_reverse_suffix (Rope* rope, uint32_t i, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach_reverse(rope->node, i, rope->node->len, rope->node->len, fn, data);
}

void rope_foreach_reverse_substr (Rope* rope, uint32_t i, uint32_t j, rope_foreach_fn fn, void* data) {
    if (rope->node != NULL) node_foreach_reverse(rope->node, i, j, rope->node->len, fn, data);
}

//
// Printing.
//

static
void content_print (Content* content) {
    for (int i = 0; i < content->len; i++) {
        uint32_t c = content->chars[i];
        if (c == '\n')
            printf("\\n");
        else
            printf("%c", (char) c);
    }
}

static
void node_print (Node* node, uint32_t level) {
    // Indentation.
    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    // Node.
    if (node->type == NODE_INTERNAL) {
        printf("[INTERNAL:len=%d,lines=%d,level=%d,rem=%d,rc=%d]\n",
                node->len, node->lines, node->level, node->rem, node->rc);
        for (int i = 0; i < node->count; i++) {
            node_print(node->child[i], level + 1);
        }
    } else if (node->type == NODE_CONTENT) {
        printf("[CONTENT:len=%d,lines=%d,level=%d,rem=%d,rc=%d|",
                node->len, node->lines, node->level, node->rem, node->rc);
        content_print(node->content);
        printf("]\n");
    }
}

void rope_print (Rope* rope) {
    if (rope->node == NULL) {
        printf("<empty>\n");
    } else {
        node_print(rope->node, 0);
    }
}
