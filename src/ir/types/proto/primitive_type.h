//-----------------------------------------------------------------------------
// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//-----------------------------------------------------------------------------
#ifndef SRC_IR_TYPES_PROTO_PRIMITIVE_TYPE_H_
#define SRC_IR_TYPES_PROTO_PRIMITIVE_TYPE_H_

#include "src/ir/proto/raksha_type.pb.h"
#include "src/ir/types/primitive_type.h"
#include "src/ir/types/type_factory.h"

namespace raksha::ir::types::proto {

using type::proto::PrimitiveTypeProto;
using type::proto::TypeProto;

// Decodes the given `primitive_type_proto` as a PrimitiveType.
Type decode(TypeFactory& type_factory,
            const PrimitiveTypeProto& primitive_type_proto);

// Encodes the given `primitive_type` as an PrimitiveTypeProto.
PrimitiveTypeProto encode(const PrimitiveType& primitive_type);

// Returns a TypeProto with the given `primitive_type`.
TypeProto encodeAsTypeProto(const PrimitiveType& primitive_type);

}  // namespace raksha::ir::types::proto

#endif  // SRC_IR_TYPES_PROTO_PRIMITIVE_TYPE_H_
