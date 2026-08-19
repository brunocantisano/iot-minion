FROM node:16-alpine AS build
WORKDIR /app

ARG REACT_APP_API_MINION_TOKEN
ARG REACT_APP_URL
ENV REACT_APP_API_MINION_TOKEN=$REACT_APP_API_MINION_TOKEN
ENV REACT_APP_URL=$REACT_APP_URL

COPY package.json ./
RUN npm install --legacy-peer-deps

COPY . .
RUN npm run build

FROM nginx:1.27-alpine
COPY --from=build /app/build /usr/share/nginx/html
COPY nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
