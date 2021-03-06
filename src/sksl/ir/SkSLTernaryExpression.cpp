/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"

namespace SkSL {

std::unique_ptr<Expression> TernaryExpression::Make(const Context& context,
                                                    std::unique_ptr<Expression> test,
                                                    std::unique_ptr<Expression> ifTrue,
                                                    std::unique_ptr<Expression> ifFalse) {
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test || !ifTrue || !ifFalse) {
        return nullptr;
    }
    int offset = test->fOffset;
    const Type* trueType;
    const Type* falseType;
    const Type* resultType;
    Operator equalityOp(Token::Kind::TK_EQEQ);
    if (!equalityOp.determineBinaryType(context, ifTrue->type(), ifFalse->type(),
                                        &trueType, &falseType, &resultType) ||
        trueType != falseType) {
        context.fErrors.error(offset, "ternary operator result mismatch: '" +
                                      ifTrue->type().displayName() + "', '" +
                                      ifFalse->type().displayName() + "'");
        return nullptr;
    }
    if (trueType->componentType().isOpaque()) {
        context.fErrors.error(offset, "ternary expression of opaque type '" +
                                      trueType->displayName() + "' not allowed");
        return nullptr;
    }
    if (context.fConfig->strictES2Mode() && trueType->isOrContainsArray()) {
        context.fErrors.error(offset, "ternary operator result may not be an array (or struct "
                                      "containing an array)");
        return nullptr;
    }
    ifTrue = trueType->coerceExpression(std::move(ifTrue), context);
    if (!ifTrue) {
        return nullptr;
    }
    ifFalse = falseType->coerceExpression(std::move(ifFalse), context);
    if (!ifFalse) {
        return nullptr;
    }
    if (test->is<BoolLiteral>()) {
        // static boolean test, just return one of the branches
        if (test->as<BoolLiteral>().value()) {
            return ifTrue;
        } else {
            return ifFalse;
        }
    }
    return std::make_unique<TernaryExpression>(offset,
                                               std::move(test),
                                               std::move(ifTrue),
                                               std::move(ifFalse));
}

}  // namespace SkSL
