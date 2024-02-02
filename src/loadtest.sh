#!/bin/bash


REQUEST_COUNT=3500
URL="http://localhost:8080/index.html"

make_request() {
    curl -s "$URL" > /dev/null
    echo "Request completed"
}

start_time=$(date +%s)

for i in $(seq 1 $REQUEST_COUNT); do
    make_request &
done
# asteptam sa se terimne background process urile
wait

end_time=$(date +%s)

total_time=$((end_time - start_time))

echo "All requests completed"
echo "Total time taken: $total_time seconds"

