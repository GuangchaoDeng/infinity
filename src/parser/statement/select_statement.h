//
// Created by jinhai on 23-2-22.
//

#pragma once

#include "parser/parsed_expr/constant_expr.h"
#include "parser/parsed_expr/column_expr.h"
#include "parser/base_statement.h"
#include "parser/table_reference/table_reference.h"
#include "parser/table_reference/cross_product_reference.h"
#include "parser/table_reference/join_reference.h"

namespace infinity {

class SelectStatement;

struct WithExpr {
    ~WithExpr() {
        if(statement_ != nullptr) {
            delete statement_;
        }
    }
    String alias_{};
    BaseStatement* statement_{};
};

enum OrderType {
    kAsc,
    kDesc
};

struct OrderByExpr {
    ~OrderByExpr() {
        delete expr_;
    }
    ParsedExpr* expr_{};
    OrderType type_{OrderType::kAsc};
};

class SelectStatement : public BaseStatement {
public:
    SelectStatement() : BaseStatement(StatementType::kSelect) {}

    ~SelectStatement() final;

    [[nodiscard]] String
    ToString() const final;

    BaseTableReference* table_ref_{nullptr};
    Vector<ParsedExpr*>* select_list_{nullptr};
    bool select_distinct_{false};
    ParsedExpr* where_expr_{nullptr};
    Vector<ParsedExpr*>* group_by_list_{nullptr};
    ParsedExpr* having_expr_{nullptr};
    Vector<OrderByExpr*>* order_by_list{nullptr};
    ParsedExpr*         limit_expr_{nullptr};
    ParsedExpr*         offset_expr_{nullptr};
    Vector<WithExpr*>*  with_exprs_{nullptr};
};

}
