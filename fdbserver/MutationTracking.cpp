/*
 * MutationTracking.cpp
 *
 * This source file is part of the FoundationDB open source project
 *
 * Copyright 2013-2020 Apple Inc. and the FoundationDB project authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>
#include "fdbserver/MutationTracking.h"
#include "fdbserver/LogProtocolMessage.h"

#if defined(FDB_CLEAN_BUILD) && MUTATION_TRACKING_ENABLED
#error "You cannot use mutation tracking in a clean/release build."
#endif

StringRef debugKey = LiteralStringRef( "\xff/globals/lastEpochEnd" );
StringRef debugKey2 = LiteralStringRef( "\xff\xff\xff\xff" );

TraceEvent debugMutationEnabled( const char* context, Version version, MutationRef const& mutation ) {
	if ((mutation.type == mutation.ClearRange || mutation.type == mutation.DebugKeyRange) &&
			((mutation.param1<=debugKey && mutation.param2>debugKey) || (mutation.param1<=debugKey2 && mutation.param2>debugKey2))) {
		return std::move(TraceEvent("MutationTracking").detail("At", context).detail("Version", version).detail("MutationType", typeString[mutation.type]).detail("KeyBegin", mutation.param1).detail("KeyEnd", mutation.param2));
	} else if (mutation.param1 == debugKey || mutation.param1 == debugKey2) {
		return std::move(TraceEvent("MutationTracking").detail("At", context).detail("Version", version).detail("MutationType", typeString[mutation.type]).detail("Key", mutation.param1).detail("Value", mutation.param2));
	} else {
		return std::move(TraceEvent());
	}
}

TraceEvent debugKeyRangeEnabled( const char* context, Version version, KeyRangeRef const& keys ) {
	if (keys.contains(debugKey) || keys.contains(debugKey2)) {
		return std::move(debugMutation(context, version, MutationRef(MutationRef::DebugKeyRange, keys.begin, keys.end) ));
	} else {
		return std::move(TraceEvent());
	}
}

#if MUTATION_TRACKING_ENABLED
TraceEvent debugMutation( const char* context, Version version, MutationRef const& mutation ) {
	return debugMutationEnabled( context, version, mutation );
}
TraceEvent debugKeyRange( const char* context, Version version, KeyRangeRef const& keys ) {
	return debugKeyRangeEnabled( context, version, keys );
}
#else
TraceEvent debugMutation( const char* context, Version version, MutationRef const& mutation ) { return std::move(TraceEvent()); }
TraceEvent debugKeyRange( const char* context, Version version, KeyRangeRef const& keys ) { return std::move(TraceEvent()); }
#endif
