FROM python:3.12-slim AS kernel-build

RUN apt-get update \
    && apt-get install -y --no-install-recommends gcc libc6-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY dynamic-site/kernel_bridge.c /build/kernel_bridge.c
COPY saem-unified/core/c17/cos_rat.c /build/cos_rat.c
COPY saem-unified/core/c17/cos_rat.h /build/cos_rat.h
COPY saem-unified/core/c17/cos_status.h /build/cos_status.h
RUN gcc -std=c17 -O2 -Wall -Wextra -Werror -pedantic -fPIC -shared \
    -I/build /build/kernel_bridge.c /build/cos_rat.c -o /build/libsaemkernel.so

FROM python:3.12-slim

ENV PORT=8080 \
    SAEM_KERNEL_LIBRARY=/app/libsaemkernel.so \
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1

WORKDIR /app
RUN groupadd --system saem \
    && useradd --system --gid saem --home-dir /app saem \
    && mkdir -p /app/neet_exam /app/jee_exams /app/jeeadv_exams /app/dynamic-site/data \
    && chown -R saem:saem /app

COPY --from=kernel-build --chown=saem:saem \
    /build/libsaemkernel.so /app/libsaemkernel.so
COPY --chown=saem:saem dynamic-site/server.py dynamic-site/inventory_exam_papers.py \
    /app/dynamic-site/
COPY --chown=saem:saem dynamic-site/web/ /app/dynamic-site/web/

USER saem
EXPOSE 8080
CMD ["python", "dynamic-site/server.py", "--host", "0.0.0.0"]
