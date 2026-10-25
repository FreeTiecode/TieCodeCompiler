SPHINXBUILD ?= ../.venv/bin/sphinx-build
SOURCEDIR = .
BUILDDIR = _build

.PHONY: help html dummy linkcheck clean

help:
	@echo "可用目标："
	@echo "  html       构建 HTML 文档（警告视为错误）"
	@echo "  dummy      仅解析，不生成输出"
	@echo "  linkcheck  检查外部/内部链接"
	@echo "  clean      清理构建目录"

html:
	$(SPHINXBUILD) -W --keep-going -b html "$(SOURCEDIR)" "$(BUILDDIR)/html"

dummy:
	$(SPHINXBUILD) -W --keep-going -b dummy "$(SOURCEDIR)" "$(BUILDDIR)/dummy"

linkcheck:
	$(SPHINXBUILD) -b linkcheck "$(SOURCEDIR)" "$(BUILDDIR)/linkcheck"

clean:
	rm -rf "$(BUILDDIR)"
