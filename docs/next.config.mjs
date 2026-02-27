import { createMDX } from 'fumadocs-mdx/next';

const withMDX = createMDX();

/** @type {import('next').NextConfig} */
const config = {
  output: 'export',
  basePath: process.env.GITHUB_PAGES ? '/RakNet' : '',
  images: { unoptimized: true },
  reactStrictMode: true,
  serverExternalPackages: ['@takumi-rs/image-response'],
};

export default withMDX(config);
