import type { BaseLayoutProps } from 'fumadocs-ui/layouts/shared';

export const gitConfig = {
  user: 'facebookarchive',
  repo: 'RakNet',
  branch: 'master',
};

export function baseOptions(): BaseLayoutProps {
  return {
    nav: {
      title: 'RakNet',
    },
    githubUrl: `https://github.com/${gitConfig.user}/${gitConfig.repo}`,
  };
}
