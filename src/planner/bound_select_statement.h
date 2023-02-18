//
// Created by jinhai on 23-1-18.
//

#pragma once

#include "planner/bound/table_ref.h"
#include "bound_statement.h"
#include "common/types.h"

namespace infinity {

struct BoundSelectStatement : public BoundStatement {
public:
    static inline SharedPtr<BoundSelectStatement>
    Make() {
        return MakeShared<BoundSelectStatement>();
    }

public:
    SharedPtr<LogicalNode>
    BuildPlan(const SharedPtr<BindContext>& bind_context) override;

    SharedPtr<LogicalNode>
    BuildFrom(SharedPtr<TableRef>& table_ref, const SharedPtr<BindContext>& bind_context);

    SharedPtr<LogicalNode>
    BuildBaseTable(SharedPtr<TableRef>& table_ref, const SharedPtr<BindContext>& bind_context);

    SharedPtr<LogicalNode>
    BuildSubqueryTable(SharedPtr<TableRef>& table_ref, const SharedPtr<BindContext>& bind_context);

    SharedPtr<LogicalNode>
    BuildCrossProductTable(SharedPtr<TableRef>& table_ref, const SharedPtr<BindContext>& bind_context);

    SharedPtr<LogicalNode>
    BuildJoinTable(SharedPtr<TableRef>& table_ref, const SharedPtr<BindContext>& bind_context);

    SharedPtr<LogicalNode>
    BuildDummyTable(SharedPtr<TableRef>& table_ref, const SharedPtr<BindContext>& bind_context);

    SharedPtr<LogicalNode>
    BuildFilter(SharedPtr<LogicalNode> root, Vector<SharedPtr<BaseExpression>>& conditions, const SharedPtr<BindContext>& bind_context);

public:
    // From clause
    SharedPtr<TableRef> table_ref_ptr_{};

    // Where conditions
    Vector<SharedPtr<BaseExpression>> where_conditions_{};

    // Aggregate expression
    Vector<SharedPtr<BaseExpression>> aggregate_expressions_{};

    // Group by expression list
    Vector<SharedPtr<BaseExpression>> group_by_expressions_{};

    // Having expression list
    Vector<SharedPtr<BaseExpression>> having_expressions_{};

    // Project expression list
    Vector<SharedPtr<BaseExpression>> projection_expressions_{};

    // Order by expression list
    Vector<SharedPtr<BaseExpression>> order_by_expressions_{};
    Vector<OrderByType> order_by_types_{};

    // limit expression
    SharedPtr<BaseExpression> limit_expression_{};
    SharedPtr<BaseExpression> offset_expression_{};

    // Pruned output expression list
    Vector<SharedPtr<BaseExpression>> pruned_expression_{};

    // Output name
    Vector<String> output_names_{};

    bool distinct_ = false;

//    SharedPtr<LogicalNode>
//    BuildPlan() override {
//        return this->logical_plan_;
//    }

    u64 groupby_index_{0};
    u64 aggregate_index_{0};
    u64 projection_index_{0};
    u64 result_index_{0};
};

}