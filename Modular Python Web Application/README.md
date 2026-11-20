# Modular Python Web Application

## Features

- **Application Factory** (`create_app()`) — safe for testing and multiple configs
- **Blueprint-based controllers** — one file per resource, explicit registration
- **Environment-based configuration** — `Development`, `Testing`, `Production` config classes
- **POST/Redirect/GET** form handling — prevents double-submit on refresh
- **Jinja2 macro library** — shared `render_error` and helpers in `layout/macros.html`
- **Test suite** — 96% coverage out of the box with pytest + pytest-cov
- **Docker-ready** — optimised multi-layer `Dockerfile` and `docker-compose.yml`

---

## Project structure

```
flask-mvc/
├── project/
│   ├── __init__.py          # create_app() factory
│   ├── config.py            # Dev / Testing / Production config classes
│   ├── controllers/
│   │   └── printer.py       # printer Blueprint + form
│   ├── models/
│   │   └── printer.py       # Printer model
│   ├── static/css/
│   └── templates/
│       ├── layout/
│       │   ├── layout.html  # base template
│       │   └── macros.html  # reusable Jinja2 macros
│       └── printer/
├── tests/
│   ├── conftest.py          # app / client fixtures
│   └── test_printer.py
├── .env.example             # required env vars — copy to .env before running
├── Dockerfile
├── docker-compose.yml
├── requirements.txt
├── requirements-test.txt
└── runserver.py
```

---
